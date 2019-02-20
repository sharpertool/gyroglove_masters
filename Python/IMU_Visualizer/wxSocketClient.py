# TCP client example
import socket
import time
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(("localhost", 5000))

pos = 2000
for x in range(0,10):
    for y in range(0,200,16):
        client_socket.send("pos %d" % (pos+y))
        time.sleep(0.1)

'''
for x in range(0,12000,16):
    data = "pos %d" % x
    client_socket.send(data)
    time.sleep(0.010)
    print ("Sent a position..")
    #data = client_socket.recv(512)
'''

client_socket.close()
