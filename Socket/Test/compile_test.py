import subprocess

subprocess.run(['g++', 'socket_test_client.cpp', '../socket_client.cpp', '-o', 'client'])
subprocess.run(['g++', 'socket_test_server.cpp', '../socket_server.cpp', '-o', 'server'])