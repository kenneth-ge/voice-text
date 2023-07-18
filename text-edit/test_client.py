# echo-client.py

import socket

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 5010  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b"Text bla bla. hello. hi. the world is so great that i just want to say hello. \0Prompt is hello, world\0\0\0")
    
    for i in range(7):
        data = s.recv(1024)
        print(f"Received {data!r}")