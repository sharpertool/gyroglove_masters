#!/usr/bin/env python

import serial
import platform
from struct import *
import numpy as np
import sys

class IMUPacket:
    def __init__(self):
        self.pkID = 0
        self.pkType = None
        self.pkBytes = 0
        self.pkData = []
        self.WatchDog = 0
        self.pkCRC = 0
        self.lenPerIMU = 9
        self.numIMUs = 6 # Default value.

        ''' Numpy transform matrices '''
        self.handgyro = np.array([[ 0,-1,0],[-1,0,0],[0,0,-1]])
        self.handacc =  np.array([[-1, 0,0],[ 0,1,0],[0,0,-1]])

        self.finggyro = np.array([[0,1,0],[1,0,0],[0,0,-1]])
        self.fingacc =  np.array([[1,0,0],[0,-1,0],[0,0,-1]])

    def RawData(self):
        if self.pkType == 0xB7:
            numIMUs = len(self.pkData)/(self.lenPerIMU*2)
            numVals = (len(self.pkData))/2
            return self.pkData

    def IMUIndexes(self,nIMUs,lenPerIMU):
        i = []
        for x in range(0,nIMUs):
            offset=x*lenPerIMU
            [i.append(y) for y in range(offset+2,offset+8)]
        return i

    def IMUData(self):
        """
        This data is for the Python capture and store routine.
        I return the data back as a python array of 6 values
        per IMU. This routine is the same as "MeasuredData", except
        that measured data returns back a packed array.
        """
        values = self.PacketToArray();
        if values:
            '''
            I want to re-pack the data. Currently we have
            a sentinal, temp, 3* gyro, 3* acc, bogus
            Lets get rid of the two sentinals and temp to just get
            3*gyro and 3* acc

            I need to reorganize the data to get the axis in their propper
            order.
            '''


            indexes = self.IMUIndexes(self.numIMUs,self.lenPerIMU)
            newVals = [values[i] for i in indexes]

            imuarray = np.array(newVals ).reshape(self.numIMUs,6)

            hand = imuarray[0]
            t = hand.reshape(2,3)
            t[0] = self.handgyro.dot(t[0])
            t[1] = self.handacc.dot(t[1])
            imuarray[0] = t.ravel()

            for x in range(1,self.numIMUs):
                imu = imuarray[x]
                t = imu.reshape(2,3)
                t[0] = self.finggyro.dot(t[0])
                t[1] = self.fingacc.dot(t[1])
                imuarray[x] = t.ravel()

            return imuarray
        else:
            return None

    def MeasuredData(self):
        imuarray = self.IMUData()
        if not imuarray == None:
            sPack = ">" + ''.join(["6h" for x in range(0,imuarray.size/6)])
            imuarray = imuarray.astype('int16')
            data = pack(sPack,*imuarray.ravel())
            return data
        else:
            return None

    def PacketToArray(self):
        if self.pkType == 0xB7:
            '''
            The packet data consists of a single byte "Mask", and then N 2-byte unsigned
            values.
            The Unpack will unpack as many values as there are, since the values are all 2 bytes
            and the length of the data packet is 2*#values + 1
            '''
            Values = []
            #print ("Length of packet:%d" % len(self.pkData))
            self.numIMUs = len(self.pkData)/(self.lenPerIMU*2)
            #print ("Number of IMU's:%d" % numIMUs)
            numVals = (len(self.pkData))/2
            sUnpack = ">" + ''.join(["H8h" for x in range(0,self.numIMUs)])
            (Values) = unpack(sUnpack,self.pkData)
            return Values
        else:
            print("Invalid packet type:%d" % self.pkType)
            return None

    def Results(self):
        values = self.PacketToArray()
        if values:

            '''
            At this point, the Mask will tell us which values are valid
            '''

            imuData = []

            keys = ['sentinal','temp','gx','gy','gz','ax','ay','az','footer']
            numIMUs = len(Values)/self.lenPerIMU
            for x in range(0,numIMUs):
                imu = {}
                y = 0
                for k in keys:
                    imu[k] = Values[x*self.lenPerIMU+y]
                    y = y +1
                imuData.append(imu)

            return imuData

        '''
        Only handling one type for now..
        '''
        return None

class IMUPacketRead:

    sStart, sFndS, sFndN, sFndP, sPkType, sPkID, sPkSize, sPkData, sPkCRC, sPkDone = range(0,10)

    def __init__(self):
        self.pkLoc = 0
        self.pkState = IMUPacketRead.sStart
        self.packet = IMUPacket()
        self.isValidPacket = False
        self.verbose = False

    def isValid(self):
        return self.isValidPacket

    def getChars(self,ser):

        while not self.isValidPacket and ser.inWaiting() > 0:
        #while not self.isValidPacket:
            if self.pkState == IMUPacketRead.sStart:
                byte = ser.read(1)
                if byte == 'S':
                    self.pkState = IMUPacketRead.sFndN
                    if self.verbose:
                        print ("Found S")
            elif self.pkState == IMUPacketRead.sFndN:
                byte = ser.read(1)
                if byte == 'N':
                    self.pkState = IMUPacketRead.sFndP
                    if self.verbose:
                        print ("Found N")
                else:
                    self.pkState = IMUPacketRead.sStart
                    if self.verbose:
                        print ("Returning to Start")
            elif self.pkState ==  IMUPacketRead.sFndP:
                byte = ser.read(1)
                if byte == 'P':
                    self.pkState = IMUPacketRead.sPkType
                    if self.verbose:
                        print ("Found P")
                else:
                    self.pkState = IMUPacketRead.sStart
                    if self.verbose:
                        print ("Returning to Start")
            elif self.pkState == IMUPacketRead.sPkType:
                pkType = ser.read(1)
                self.packet.pkType = unpack('>B',pkType)[0]
                self.pkState = IMUPacketRead.sPkID
                if self.verbose:
                    print ("Found PkType:0x%x" % self.packet.pkType)
            elif self.pkState == IMUPacketRead.sPkID:
                ID = ser.read(1)
                self.packet.pkID = unpack('>B',ID)[0]
                self.pkState = IMUPacketRead.sPkSize
                if self.verbose:
                    print ("Found pkID:0x%x" % self.packet.pkID)
            elif self.pkState == IMUPacketRead.sPkSize:
                pkBytes = ser.read(1)
                self.packet.pkBytes = unpack('>B',pkBytes)[0]
                if self.verbose:
                    print("PkBytes:0x%x" % self.packet.pkBytes)
                if self.packet.pkBytes > 0:
                    '''
                    Doing a direct read here, rather than defering the read and
                    reading a seperate bytes... should be mucho faster.
                    '''
                    if self.verbose:
                        print ("Reading %d Bytes" % self.packet.pkBytes)
                    try:
                        #self.packet.pkData = []
                        #for x in range(0,self.packet.pkBytes):
                        #    b = ser.read(1)
                        #    self.packet.pkData.append(unpack('>B',b)[0])
                        d = ser.read(self.packet.pkBytes)
                        self.packet.pkData = d
                        self.pkState = IMUPacketRead.sPkCRC
                        #self.pkState = IMUPacketRead.sPkDone
                        if self.verbose:
                            print ("Going to PkDone")
                    except:
                        '''
                        What happens if I time out... want to handle this someday
                        '''
                        self.pkState = IMUPacketRead.sStart
                        print("Exception while reading packet data")
                else:
                    self.pkState = IMUPacketRead.sPkDone
                    if self.verbose:
                        print ("Packet Size == 0, Returning to Start")
            elif self.pkState == IMUPacketRead.sPkCRC:
                pkWD = ser.read(1)
                self.packet.WatchDog = unpack('>B',pkWD)[0]
                pkCRC = ser.read(2)
                self.packet.pkCRC = unpack('>H',pkCRC)[0]
                if self.verbose:
                    print("CRC Code:0x%x" % self.packet.pkCRC)
                self.pkState = IMUPacketRead.sPkDone
                if self.verbose:
                    print ("Going to PkDone")

            if self.pkState == IMUPacketRead.sPkDone:
                self.isValidPacket = True

        return self.isValidPacket

    def Clear(self):
        self.isValidPacket = False
        self.packet = IMUPacket()
        self.pkState = IMUPacketRead.sStart
