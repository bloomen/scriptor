import socket
import logging

s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/server.sock")
while True:
    s.send(input(">>>").encode())
