import subprocess
import platform

is_windows = platform.system() == "Windows"
ws2_lib = ['-lws2_32'] if is_windows else []

# Client build
subprocess.run([
    'g++', 
    'socket_test_client.cpp', 
    'csv_sender.cpp', 
    'socket_client.cpp', 
    '-o', 'client'
] + ws2_lib)

# Server build
subprocess.run([
    'g++', 
    'socket_test_server.cpp', 
    'csv_receiver.cpp', 
    'socket_server.cpp', 
    '-o', 'server'
] + ws2_lib)
