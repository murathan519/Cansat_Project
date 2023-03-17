import socket
import time
host = "localhost"
port = 2300
import random

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("Socket created")

    s.bind((host, port))
    print("Socket connected to port {}".format(port))

    s.listen()
    # Client connection
    print("Socket is being listened")
    c, addr = s.accept()
    print('Upcoming Connection:', addr)
    file = open("C:\\Users\\berfin\\OneDrive\\Desktop\\pythontopython\\tlmsd.txt", "r", encoding="utf-8")

except socket.error as msg:
    print("Error:", msg)
    i = 0

while True:
    message = file.readline()
    print(message)
    c.send(message.encode('utf-8'))
    time.sleep(1)
    c.close()