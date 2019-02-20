#!/usr/bin/env python

import sys
import serial
import platform
import glob
import time
import re
import os.path
from optparse import OptionParser
import numpy as np
import scipy as sp
import scipy.io

from IMUPacket import *
from GloveAPI import GloveAPI

class GloveChar(object):

    def __init__(self):
        super(GloveChar,self).__init__()

        self.api = None
        self.rate = 100


    def printStatus(self,s):
        sys.stdout.write("\b"*5 + "%5s" % s)

    def InitAPI(self,rate):

        self.rate = rate

        print("Get API")
        self.api = GloveAPI()
        print("Init HW")
        self.api.initHardware()
        print("Get Baud")
        self.api.getBaud()
        self.api.rate(rate)

    def showPacket(self,id,imudata):
        s = "\t".join([str(x) for x in imudata.ravel()])
        print("Id:%d %s" % (id,s))

    def initSequence(self):
        self.api.clearIMUPacketEngine()

        self.api.streamstart(True)
        self.api.StreamWD()

    def endSequence(self):
        self.api.streamstop()

    def contiueSequence(self,count):
        """
        This assumes the sequence has already been started.
        """

        data = np.zeros((count,1+6*6))
        self.api.StreamWD()

        print "Percent Complete:",
        for x in range(0,count):

            self.api.StreamWD()
            p = self.api.getIMUPacket()

            '''
            Reset the watchdog each time. No reason not to since the serial link to the
            unit is not used for anything except this.
            '''
            self.api.StreamWD()

            if p:

                '''
                Append the data to my potential Matlab array
                '''
                imudata = p.IMUData()
                if not imudata == None:
                    #self.showPacket(p.pkID, imudata)
                    imuidx = 0
                    data[x][0] = p.pkID
                    data[x][1:] = imudata.ravel()

                    if x and x % 100 == 0:
                        sys.stdout.write(".")
                        sys.stdout.flush()

                else:
                    print ("IMU Data returned")
            else:
                print "Bad Packet"

        print("Done")
        return data

    def captureSequence(self,count):

        self.initSequence()

        data = self.contiueSequence(count)

        self.endSequence()
        return data

    def logData(self,data,destDir,filePrefix,nRunIndex,rate):

        fname = "%s%02d.mat" % (filePrefix,nRunIndex)
        outfile = os.path.join(destDir,fname)

        if os.path.isfile(outfile):
            print ("Destination file exists")
            os.unlink(outfile)

        print "Saving data to %s" % outfile
        sp.io.savemat(outfile, mdict={
            'Data': data,
            'Name':"CharRun%d" % nRunIndex,
            'T':1.0/float(rate)})

    def runLoop(self,runDelay,nLoops,nStart,nPerRun,destDir,filePrefix):

        for nRunIndex in range(nStart,nLoops+1):
            data = self.captureSequence(nPerRun)

            if data == None:
                # No data returned!!
                return

            """
            Good data returned. Log it to a file and then
            delay for the next sequence.
            """

            self.logData(data,destDir,
                         filePrefix,
                         nRunIndex,
                         self.rate)

            if runDelay:
                self.Delay(runDelay)

    def Delay(self,dly):
        """
        Delay time in minutes. Wake up every 5 minutes and print something.
        """

        totalDelay = 0
        while totalDelay < dly:
            currDelay = min(5,dly - totalDelay)
            totalDelay = totalDelay + currDelay
            print("Delay time left:%d" % (dly - totalDelay))
            time.sleep(currDelay*60)

        print("Delay Done")

def main():
    usage = "usage: %prog -n -m -s]"
    parser = OptionParser(usage)
    parser.add_option("-r", dest="rate",
                      action="store",
                      type="int",
                      default=100,
                      help="Sample Rate")
    parser.add_option("-c", dest="count",
                      action="store",
                      type="int",
                      default=6000,
                      help="Number to read.")
    parser.add_option("-n", dest="number",
                      action="store",
                      type="int",
                      default=60,
                      help="Number of cycles.")
    parser.add_option("-s", dest="numstart",
                      action="store",
                      type="int",
                      default=1,
                      help="Start Number.")
    parser.add_option("-t", dest="timedelay",
                      action="store",
                      type="int",
                      default=60,
                      help="Time delay in minutes between runs.")
    parser.add_option("-d",dest="path",
                       action="store",
                       type="string",
                       default="GloveCharData",
                       help="Directory to output debug data to")
    parser.add_option("-p",dest="prefix",
                       action="store",
                       type="string",
                       default="GloveChar",
                       help="File prefix to output debug data to")

    parser.add_option("-v",action="store_false",dest="verbose")

    (options, args) = parser.parse_args()

    obj = GloveChar()

    obj.InitAPI(options.rate)
    obj.runLoop(options.timedelay,
                options.number,
                options.numstart,
                options.count,
                options.path,
                options.prefix)

if __name__ == "__main__":
    main()
