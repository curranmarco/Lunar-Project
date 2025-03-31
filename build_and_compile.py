import subprocess

# Output binaries
client_binary = "client_exec"
server_binary = "server_exec"

# Common include paths
include_paths = [
    "-IPacket",
    "-ISensors",
    "-ISocket",
    "-ILocation_Map"
]

# Compilation commands
client_compile = [
    "g++", "-std=c++17",
    *include_paths,
    "Packet/packet_functions.cpp",
    "Sensors/Sensors.cpp",
    "Location_Map/location_map.cpp",
    "Socket/socket_client.cpp",
    "client_main.cpp",
    "-o", client_binary
]

server_compile = [
    "g++", "-std=c++17",
    *include_paths,
    "Packet/packet_functions.cpp",
    "Location_Map/location_map.cpp",
    "Socket/socket_server.cpp",
    "server_main.cpp",
    "-o", server_binary
]

def compile_target(name, command):
    print(f"🔧 Compiling {name}...")

    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    if result.returncode != 0:
        print(f"Compilation of {name} failed:\n")
        print(result.stderr)
    else:
        print(f"{name} compiled successfully! Binary: ./{name}")

def main():
    compile_target(client_binary, client_compile)
    compile_target(server_binary, server_compile)

if __name__ == "__main__":
    main()

