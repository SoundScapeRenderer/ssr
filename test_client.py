import socket


TCP_IP = '127.0.0.1'
TCP_PORT = 4711
BUFFER_SIZE = 1024
MESSAGE = b"<request><reference><orientation azimuth='45'/></reference></request> \n"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
s.send(MESSAGE)

for t in range(10):
    data = s.recv(BUFFER_SIZE)
    print ("received data:", data)

s.close()
