import os
import subprocess

# Compilation settings
cpp_files = [
    "client_main.cpp",
    "Packet/packet_functions.cpp",
    "Socket/socket_client.cpp",
    "Socket/socket_server.cpp"
]

# Output executable name
output_exec = "actuator_client"

# Command
compile_cmd = ["g++", "-std=c++17", "-o", output_exec] + cpp_files

# Include directories (assuming relative paths)
include_dirs = ["-I.", "-IPacket", "-ISocket"]
compile_cmd += include_dirs

# Run the compilation
print("🔧 Compiling actuator client...")
result = subprocess.run(compile_cmd, capture_output=True, text=True)

if result.returncode == 0:
    print("✅ Compilation successful.")
    print("🚀 Running actuator client...\n")
    subprocess.run([f"./{output_exec}"])
else:
    print("❌ Compilation failed:")
    print(result.stderr)