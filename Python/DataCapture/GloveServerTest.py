#!/usr/bin/env python

# Echo client program
import socket
import time
from struct import *

HOST = '192.168.1.147'    # The remote host
PORT = 5120              # The same port as used by the server

keys = ['sentinal','temp','gx','gy','gz','ax','ay','az','footer']
fkeys = ['temp','gx','gy','gz','ax','ay','az']

def showPacket(imudata,keys):
    strings = []
    for i in imudata:
        strings.append("\t".join(["%x" % i[k] for k in keys]))
    print("\t".join(strings))

def printPacket(f,fkeys,p):
    imudata = p.Results()
    strings = []
    for i in imudata:
        strings.append("\t".join(["%d" % i[k] for k in fkeys]))
    f.write("\t".join(strings) + "\n")

def Results(pkData):
    '''
    The packet data consists of a single byte "Mask", and then N 2-byte unsigned
    values.
    The Unpack will unpack as many values as there are, since the values are all 2 bytes
    and the length of the data packet is 2*#values + 1
    '''
    lenPerIMU = 9
    Values = []
    #print ("Length of packet:%d" % len(self.pkData))
    numIMUs = len(pkData)/(lenPerIMU*2)
    #print ("Number of IMU's:%d" % numIMUs)
    numVals = (len(pkData))/2
    sUnpack = ">" + ''.join(["H8h" for x in range(0,numIMUs)])
    (Values) = unpack(sUnpack,pkData)
    #for x in self.pkData:
    #    print("Data:0x%x" % x)

    '''
    At this point, the Mask will tell us which values are valid
    '''

    imuData = []

    keys = ['sentinal','temp','gx','gy','gz','ax','ay','az','footer']
    numIMUs = len(Values)/lenPerIMU
    for x in range(0,numIMUs):
        imu = {}
        y = 0
        for k in keys:
            imu[k] = Values[x*lenPerIMU+y]
            y = y +1
        imuData.append(imu)

    return imuData


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.send('quit')
sys.exit(1)

time.sleep(0.1)

#s.send('stop')


for x in range(0,100):
    s.send("data")
    d = s.recv(1024)

    imudata = Results(d)

    showPacket(imudata,keys)

s.close()
