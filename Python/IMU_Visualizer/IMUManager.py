#!/usr/bin/env python

import serial
import platform
import glob
import time
import re
import numpy
from GloveAPI import GloveAPI

class IMUManager(object):
    def __init__(self,api):
        super(IMUManager,self).__init__()

        self.api = api

    def Configure(self):
        pass

    def StreamOn(self,bOn = True):
        self.api.stream(bOn)

    def StreamCont(self,bOn = True):
        if bOn:
            print("Started streaming..")
            self.api.sendPacket(b'streamcont 1\n')
        else:
            print("Ended streaming..")
            self.api.sendPacket(b'streamcont 0\n')

    def ReadIMUData(self):
        '''
        Need to read the current state of the streams..
        Need to turn on streaming first of course.

        '''
        data = self.api.getIMUPacket()
        return data

if __name__ == "__main__":
    api = GloveAPI()
    api.initHardware()

    print ("Done")
