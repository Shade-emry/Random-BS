#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include <vector>

// Structure to hold sensor data (e.g., from an IMU)
struct SensorData {
    float accelX;
    float accelY;
    float accelZ;
    // Additional fields (like orientation quaternion) can be added if needed.
};

class DataProcessor {
public:
    // Corrects acceleration axes to match the expected orientation frame.
    // This function rotates the raw acceleration vector by 90° about the X-axis.
    static SensorData correctAcceleration(const SensorData &rawData);
};

#endif // DATA_PROCESSOR_H
