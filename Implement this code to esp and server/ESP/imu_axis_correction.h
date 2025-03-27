#ifndef IMU_AXIS_CORRECTION_H
#define IMU_AXIS_CORRECTION_H

// Structure to hold IMU data (gyroscope, accelerometer, etc.)
struct IMUData {
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    // Additional fields (e.g. orientation quaternion) can be added if needed.
};

class IMUAxisCorrection {
public:
    // Applies axis remapping (for accelerometer) and real-time temperature compensation
    // for the gyroscope bias.
    // Parameters:
    //   rawData - raw sensor data (gyro in °/s, accel in m/s^2)
    //   rawTemp - raw temperature value from the BNO08x (Report ID 0x15, bytes 10-11)
    // Returns:
    //   IMUData with corrected sensor values.
    static IMUData applyCorrection(const IMUData &rawData, int16_t rawTemp);
};

#endif // IMU_AXIS_CORRECTION_H
