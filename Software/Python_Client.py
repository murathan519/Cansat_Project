import socket
import time
host = "192.168.137.124" 		#RPI IP address
port = 8888
import random

try:
    s = socket.socket()			#Socket creation
    s.connect((host, port))		#Connection
    filee = open("/home/pi/Desktop/TURKSAT/telemetry.txt", "r+", encoding="utf-8")

except socket.error as msg:
    print("Error:", msg)
i = 0

while True:
    message = filee.readline()
    print(message)
    s.send(message.encode('utf-8'))
    time.sleep(.5)

s.close()