# SlimeVR Server Code Fix Package

This package provides updated sensor data processing files for the SlimeVR Server.
It corrects acceleration data alignment by rotating the vector to match the proper sensor orientation.

## Files:
- **data_processor.h / data_processor.cpp:**  
  Implements a correction function to rotate the acceleration vector by 90° about the X-axis.
- **changelog.md:**  
  Describes the changes made.
- **README.md:**  
  Provides integration instructions.

## Integration Instructions:
1. Replace your existing `data_processor.*` files with these.
2. Ensure that sensor data received by the server is processed via `DataProcessor::correctAcceleration()`
   before being used in sensor fusion.
3. Follow the changelog for further details.
