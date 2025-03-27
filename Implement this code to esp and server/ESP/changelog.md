# ESP Firmware Changelog

## Fixes and Enhancements
- **IMU Axis Remapping & Temperature Compensation:**  
  - Integrated dynamic temperature compensation into the gyro data processing.
  - Reads temperature from the BNO08x via SH-2 Report ID 0x15 (bytes 10–11), with each LSB equal to 0.5 K and 0 corresponding to 23°C.
  - Uses a linear model (default coefficient: 0.015 °/s per °C) to adjust gyro bias in real time.
  - Applies axis remapping for the accelerometer (swapping Y and Z, with inversion of the new Z).
  - Updates the VQF filter with bias-corrected gyro data (converted to rad/s) for improved 9DoF orientation estimation.
- **Optional Debug Reporting:**  
  - Added commented code to transmit raw temperature data to the server for debugging.

## Notes
- The compensation model initializes its baseline using the first valid temperature reading.
- Adjust the temperature coefficient as needed based on empirical calibration.
- This firmware is intended for use in a 9DoF room-scale VR system.
