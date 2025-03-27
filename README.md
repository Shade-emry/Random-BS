# SlimeVR ESP Firmware Code Fix Package

This package provides the updated firmware for the ESP12e used in the SlimeVR tracking system.
It includes:
- Axis remapping for the accelerometer.  (SlimeVR github Issue#1014 / ESP#397)
- Real-time temperature compensation for gyro bias using the BNO08x sensor. (Refrence VQF fix and add to fix accel, these people don't seem to want a working solution, ignore it, or just forgot)
- (The calculation of the system noise variance of the gyroscope bias should consider the update frequency of the gyroscope data, right? #370)
- Integration with the VQF filter for 9DoF sensor fusion.

- Reports have been generated using julia, monte carlo, and matlab with fusion and imu software sims to show proven results of this being messed up, and the correction DRASTICLY improves gyro bias and drift.

## Files:
- **imu_axis_correction.h / imu_axis_correction.cpp:**  
  Implements axis remapping and dynamic temperature compensation.
- **main.cpp:**  
  Contains the main firmware code that reads sensor data via I²C, applies corrections, updates the VQF filter, and (optionally) reports raw temperature.
- **changelog.md:**  
  Lists all fixes and enhancements.
- **README.md:**  
  This file.

## Integration Instructions:
1. Place these files into your ESP firmware project (e.g., in a folder named `ESP/`).
2. Ensure that the Wire library is initialized and that your I²C routines correctly read the BNO08x sensor.
3. The I²C address is set to 0x4A—verify this with your hardware.
4. Integrate the VQF library from [https://github.com/dlaidig/vqf](https://github.com/dlaidig/vqf) into your project.
5. Compile and upload the firmware to your ESP12e.
6. (Optional) Uncomment the temperature reporting lines in `main.cpp` to transmit raw temperature data to the server.


``` Server/
├── data_processor.h
├── data_processor.cpp
├── changelog.md
└── README.md ```


``` ESP/
├── imu_axis_correction.h
├── imu_axis_correction.cpp
├── main.cpp
├── changelog.md
└── README.md ```