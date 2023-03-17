import socket
import time

# Socket creation
s = socket.socket()

# Address and port
host = "192.168.137.124" #ground station IP address
port = 2300
# Create connection
s.connect((host, port))

while True:
    # Receive reply from server
    reply= s.recv(18000)
    telemetry = reply.decode("utf-8")
    print(reply.decode("utf-8"))
    
    with open('/home/pi/Desktop/TURKSAT/GCS.txt', "w", encoding="utf-8") as file:
    file.write(telemetry)

# Kill the connection
s.close()