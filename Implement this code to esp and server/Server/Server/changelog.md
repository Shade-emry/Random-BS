# Server Firmware Changelog

## Fixes
- **Axis Alignment Correction:**  
  Applied a 90° rotation about the X-axis to the incoming acceleration data so that the sensor’s raw Y and Z axes are remapped to match the expected orientation frame.
  This resolves the discrepancies reported in GitHub issues [#397](https://github.com/SlimeVR/SlimeVR-Tracker-ESP/issues/397)
  and [#1014](https://github.com/SlimeVR/SlimeVR-Server/pull/1014#issuecomment-2698353717).

## Details
- The function `DataProcessor::correctAcceleration()` in `data_processor.cpp` performs the rotation.
- Integrate this function into your sensor processing pipeline before sensor fusion.
