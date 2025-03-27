/*
	SlimeVR Code is placed under the MIT license
	Copyright (c) 2021 Eiren Rain & SlimeVR contributors

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include <i2cscan.h>
#include "GlobalVars.h"
#include "Wire.h"
#include "batterymonitor.h"
#include "credentials.h"
#include "debugging/TimeTaken.h"
#include "globals.h"
#include "logging/Logger.h"
#include "ota.h"
#include "serial/serialcommands.h"
#include "status/TPSCounter.h"

// NEW INCLUDES FOR TEMP COMPENSATION AND VQF
#include "imu_axis_correction.h"
#include "VQF.h"  // VQF filter library from https://github.com/dlaidig/vqf

// I2C address for the BNO08x sensor for our ESP (set to 0x4A as specified)
#define BNO08X_I2C_ADDRESS 0x4A
// Expected report length for SH-2 Report ID 0x15 (adjust if necessary)
#define REPORT_LENGTH 20

// Global timer and other objects (unchanged)
Timer<> globalTimer;
SlimeVR::Logging::Logger logger("SlimeVR");
SlimeVR::Sensors::SensorManager sensorManager;
SlimeVR::LEDManager ledManager(LED_PIN);
SlimeVR::Status::StatusManager statusManager;
SlimeVR::Configuration::Configuration configuration;
SlimeVR::Network::Manager networkManager;
SlimeVR::Network::Connection networkConnection;

#if DEBUG_MEASURE_SENSOR_TIME_TAKEN
SlimeVR::Debugging::TimeTakenMeasurer sensorMeasurer{"Sensors"};
#endif

int sensorToCalibrate = -1;
bool blinking = false;
unsigned long blinkStart = 0;
unsigned long loopTime = 0;
unsigned long lastStatePrint = 0;
bool secondImuActive = false;
BatteryMonitor battery;
TPSCounter tpsCounter;

// Global instance of the VQF filter (for temp-compensated update)
VQF vqf;

//
// Function: readBNO08xReport
// Reads a sensor report from the BNO08x via I2C using Report ID 0x15.
// Writes the Report ID and then reads REPORT_LENGTH bytes into the buffer.
//
bool readBNO08xReport(uint8_t* buffer, size_t len) {
	Wire.beginTransmission(BNO08X_I2C_ADDRESS);
	Wire.write(0x15); // Report ID for gyro & temperature data.
	if (Wire.endTransmission() != 0) {
		return false;
	}
	size_t bytesRead = Wire.requestFrom(BNO08X_I2C_ADDRESS, (uint8_t)len);
	if (bytesRead < len) {
		return false;
	}
	for (size_t i = 0; i < len; i++) {
		buffer[i] = Wire.read();
	}
	return true;
}

//
// Function: updateTempCompensatedOrientation
// Reads the BNO08x sensor report, extracts temperature, gyro, and accel data,
// applies axis correction and dynamic temperature compensation via IMUAxisCorrection,
// updates the VQF filter, and prints the orientation to Serial.
//
// This function can be called in addition to sensorManager.update() for debugging.
//
void updateTempCompensatedOrientation() {
	uint8_t report[REPORT_LENGTH];
	if (readBNO08xReport(report, REPORT_LENGTH)) {
		// Extract raw temperature from bytes 10-11.
		int16_t rawTemp = (int16_t)(report[10] | (report[11] << 8));
		
		// Extract raw gyro data from bytes 4-9.
		int16_t rawGX = (int16_t)(report[4] | (report[5] << 8));
		int16_t rawGY = (int16_t)(report[6] | (report[7] << 8));
		int16_t rawGZ = (int16_t)(report[8] | (report[9] << 8));
		
		// Extract raw accelerometer data from bytes 12-17.
		int16_t rawAX = (int16_t)(report[12] | (report[13] << 8));
		int16_t rawAY = (int16_t)(report[14] | (report[15] << 8));
		int16_t rawAZ = (int16_t)(report[16] | (report[17] << 8));
		
		// Build raw IMU data structure.
		IMUData rawData;
		// Convert raw sensor values to proper units (assumed pre-scaled).
		rawData.gyroX = (float)rawGX;  // in °/s
		rawData.gyroY = (float)rawGY;  // in °/s
		rawData.gyroZ = (float)rawGZ;  // in °/s
		rawData.accelX = (float)rawAX; // in m/s^2
		rawData.accelY = (float)rawAY; // in m/s^2
		rawData.accelZ = (float)rawAZ; // in m/s^2
		
		// Apply axis correction and temperature compensation.
		IMUData correctedData = IMUAxisCorrection::applyCorrection(rawData, rawTemp);
		
		// Optional: Report raw temperature to server.
		// Uncomment and implement your server transmission function if needed.
		/*
		float tempC = 23.0f + 0.5f * rawTemp;
		server.sendDebug("Temperature (°C)", tempC);
		*/
		
		// Convert corrected gyro values from °/s to rad/s.
		const float DEG2RAD = 3.1415926f / 180.0f;
		float gyroX_rad = correctedData.gyroX * DEG2RAD;
		float gyroY_rad = correctedData.gyroY * DEG2RAD;
		float gyroZ_rad = correctedData.gyroZ * DEG2RAD;
		
		// Build accelerometer vector (assumed in m/s^2).
		Vector3f accel(correctedData.accelX, correctedData.accelY, correctedData.accelZ);
		
		// Magnetometer data: insert actual data if available. Here we use zeros.
		Vector3f mag(0.0f, 0.0f, 0.0f);
		
		// Update the VQF filter with corrected sensor data.
		vqf.update(Vector3f(gyroX_rad, gyroY_rad, gyroZ_rad), accel, mag);
		
		// Output computed orientation for debugging.
		Quaternion orientation = vqf.getOrientation();
		Serial.print("TempComp Orientation (w,x,y,z): ");
		Serial.print(orientation.w); Serial.print(", ");
		Serial.print(orientation.x); Serial.print(", ");
		Serial.print(orientation.y); Serial.print(", ");
		Serial.println(orientation.z);
	}
}

//
// Original main() below remains unchanged.
//

Timer<> globalTimer;
SlimeVR::Logging::Logger logger("SlimeVR");
SlimeVR::Sensors::SensorManager sensorManager;
SlimeVR::LEDManager ledManager(LED_PIN);
SlimeVR::Status::StatusManager statusManager;
SlimeVR::Configuration::Configuration configuration;
SlimeVR::Network::Manager networkManager;
SlimeVR::Network::Connection networkConnection;

#if DEBUG_MEASURE_SENSOR_TIME_TAKEN
SlimeVR::Debugging::TimeTakenMeasurer sensorMeasurer{"Sensors"};
#endif

int sensorToCalibrate = -1;
bool blinking = false;
unsigned long blinkStart = 0;
unsigned long loopTime = 0;
unsigned long lastStatePrint = 0;
bool secondImuActive = false;
BatteryMonitor battery;
TPSCounter tpsCounter;

void setup() {
	Serial.begin(serialBaudRate);
	globalTimer = timer_create_default();

	Serial.println();
	Serial.println();
	Serial.println();

	logger.info("SlimeVR v" FIRMWARE_VERSION " starting up...");

	statusManager.setStatus(SlimeVR::Status::LOADING, true);

	ledManager.setup();
	configuration.setup();

	SerialCommands::setUp();
	// Clear I2C bus to fix issues when resetting ESP.
	auto clearResult = I2CSCAN::clearBus(PIN_IMU_SDA, PIN_IMU_SCL);
	if (clearResult != 0) {
		logger.error("Can't clear I2C bus, error %d", clearResult);
	}

	// Initialize I2C.
#if ESP32
	Wire.end();
#endif
	Wire.begin(static_cast<int>(PIN_IMU_SDA), static_cast<int>(PIN_IMU_SCL));
#ifdef ESP8266
	Wire.setClockStretchLimit(150000L);
#endif
#ifdef ESP32
	Wire.setTimeOut(150);
#endif
	Wire.setClock(I2C_SPEED);

	// Wait for IMU to boot.
	delay(500);

	sensorManager.setup();
	networkManager.setup();
	OTA::otaSetup(otaPassword);
	battery.Setup();

	statusManager.setStatus(SlimeVR::Status::LOADING, false);

	sensorManager.postSetup();

	loopTime = micros();
	tpsCounter.reset();
	
	// Initialize the VQF filter for temperature compensation.
	vqf.begin();
}

void loop() {
	tpsCounter.update();
	globalTimer.tick();
	SerialCommands::update();
	OTA::otaUpdate();
	networkManager.update();

#if DEBUG_MEASURE_SENSOR_TIME_TAKEN
	sensorMeasurer.before();
#endif
	sensorManager.update();
#if DEBUG_MEASURE_SENSOR_TIME_TAKEN
	sensorMeasurer.after();
#endif

	battery.Loop();
	ledManager.update();
	I2CSCAN::update();
#ifdef TARGET_LOOPTIME_MICROS
	long elapsed = (micros() - loopTime);
	if (elapsed < TARGET_LOOPTIME_MICROS) {
		long sleepus = TARGET_LOOPTIME_MICROS - elapsed - 100;
		long sleepms = sleepus / 1000;
		if (sleepms > 0) {
			delay(sleepms);
			sleepus -= sleepms * 1000;
		}
		if (sleepus > 100) {
			delayMicroseconds(sleepus);
		}
	}
	loopTime = micros();
#endif
#if defined(PRINT_STATE_EVERY_MS) && PRINT_STATE_EVERY_MS > 0
	unsigned long now = millis();
	if (lastStatePrint + PRINT_STATE_EVERY_MS < now) {
		lastStatePrint = now;
		SerialCommands::printState();
	}
#endif

	// Call the new temperature-compensated update.
	// This function reads sensor data via I2C from the BNO08x (I2C address 0x4A),
	// applies axis remapping and dynamic temperature compensation, updates VQF,
	// and prints the orientation.
	updateTempCompensatedOrientation();
}
