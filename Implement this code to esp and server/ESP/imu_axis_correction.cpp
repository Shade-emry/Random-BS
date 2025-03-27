#include "imu_axis_correction.h"

namespace {
    // Baseline gyro bias (°/s) measured at baseline temperature.
    float biasX0 = 0.0f, biasY0 = 0.0f, biasZ0 = 0.0f;
    float tempBaseline = 23.0f; // Baseline temperature in °C (default 23°C)
    bool biasCalibrated = false;
    
    // Gyro temperature coefficient: change in bias (°/s) per °C.
    const float gyroTempCoeff = 0.015f; // Typical for the BMI055 gyro.
}

IMUData IMUAxisCorrection::applyCorrection(const IMUData &rawData, int16_t rawTemp) {
    IMUData corrected = rawData;
    
    // ---- Accelerometer Axis Remapping ----
    // Swap Y and Z axes and invert the new Z axis.
    float tempAccelY = rawData.accelY;
    corrected.accelY = rawData.accelZ;
    corrected.accelZ = -tempAccelY;
    
    // ---- Temperature Compensation for Gyro Bias ----
    // Convert raw temperature to °C:
    // Each LSB = 0.5°C, with raw value 0 corresponding to 23°C.
    float tempC = 23.0f + 0.5f * (int16_t)rawTemp;
    
    // If not calibrated, use the first reading as baseline.
    if (!biasCalibrated) {
        tempBaseline = tempC;
        biasX0 = 0.0f; // Optionally, set initial bias if known.
        biasY0 = 0.0f;
        biasZ0 = 0.0f;
        biasCalibrated = true;
    }
    
    float dT = tempC - tempBaseline;
    // Compute current bias offsets using a linear model.
    float biasX = biasX0 + gyroTempCoeff * dT;
    float biasY = biasY0 + gyroTempCoeff * dT;
    float biasZ = biasZ0 + gyroTempCoeff * dT;
    
    // Subtract the computed bias from the raw gyro readings.
    corrected.gyroX = rawData.gyroX - biasX;
    corrected.gyroY = rawData.gyroY - biasY;
    corrected.gyroZ = rawData.gyroZ - biasZ;
    
    return corrected;
}
