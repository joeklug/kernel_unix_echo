#!/usr/bin/env python3

import socket
import time

def main():
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.connect("/tmp/blahsock")
    s.send(b"Test\0")
    print(s.recv(1024))
    s.close()

if __name__ == "__main__":
    main()
