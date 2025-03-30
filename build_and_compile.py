# build_and_compile.py

import subprocess
import os

# Output binary name
output_binary = "sensor_test"

# Compile command using your actual folder structure
compile_command = [
    "g++",
    "-std=c++11",
    "-ISensors",              # Include path for Sensors headers
    "-Ilocation_map",         # Include path for location_map headers
    "Sensors/Sensors.cpp",    # Sensor class implementation
    "location_map/location_map.cpp",  # Location map definitions
    "Sensors/test_sensors.cpp",        # Your test file
    "-o", output_binary
]

def main():
    print("🔧 Compiling Sensor test...")

    result = subprocess.run(compile_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    if result.returncode != 0:
        print("❌ Compilation failed:\n")
        print(result.stderr)
    else:
        print(f"✅ Compilation successful! Binary created: ./{output_binary}")

if __name__ == "__main__":
    main()
