#include "data_processor.h"
#include <cmath>

// Rotate the raw acceleration vector by 90° about the X-axis:
// Maps raw Y -> raw Z and raw Z -> -raw Y.
SensorData DataProcessor::correctAcceleration(const SensorData &rawData) {
    SensorData corrected;
    corrected.accelX = rawData.accelX;
    corrected.accelY = rawData.accelZ;
    corrected.accelZ = -rawData.accelY;
    return corrected;
}
