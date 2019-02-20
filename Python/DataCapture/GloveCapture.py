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

def showPacket(id,imudata):
    s = "\t".join([str(x) for x in imudata.ravel()])
    print("Id:%d %s" % (id,s))

def AutoIncrementFile(infile):
    if os.path.isfile(infile):
        ''' We have some work to do.. to autoincrement the file '''
        dir = os.path.dirname(options.outfile)
        fullname = os.path.basename(options.outfile)
        (fname,ext) = os.path.splitext(fullname)

        '''
        List the directory, then search only for those that include the base part of the
        filename, so File File1 File2 File3, etc would all be returned
        '''

        maxIndex = 0
        for f in os.listdir(dir):
            m = re.search(fname,f)
            if m:
                (fbase,e) = os.path.splitext(f)
                m = re.search("(\d+)$",fbase) # Find trailing number
                if m:
                    idx = int(m.group(1))
                    if idx > maxIndex:
                        maxIndex = idx

        index =maxIndex + 1

        outfile = os.path.join(dir,fname + "%d" % index + ext)

        print("%s incremented to %s" %(infile,outfile))

        return outfile
    else:
        return infile


if __name__ == "__main__":

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
                      default=200,
                      help="Number to read.")
    parser.add_option("-p",dest="path",
                       action="store",
                       type="string",
                       default="DataCollection",
                       help="Directory to output debug data to")
    parser.add_option("-f",dest="outfile",
                       action="store",
                       type="string",
                       default="",
                       help="File to output debug data to")
    parser.add_option("-n",dest="dataname",
                      action="store",
                      type="string",
                      default="arr",
                      help="Name to use for the matlab data"
    )
    parser.add_option("-v",action="store_false",dest="verbose")
    parser.add_option("-s",action="store_true",dest="stop")

    (options, args) = parser.parse_args()

    if options.outfile == "":
        options.outfile = os.path.join(options.path,options.dataname + ".mat")

    #outfile = AutoIncrementFile(options.outfile)
    outfile = options.outfile

    print("Get API")
    api = GloveAPI()
    print("Init HW")
    api.initHardware()
    print("Get Baud")
    api.getBaud()

    if options.stop:
        api.streamstop()
        sys.exit(0)

    numToRead = options.count

    if options.outfile:
        writeFile = True
    else:
        writeFile = False

    api.clearIMUPacketEngine()
    keys = ['gx','gy','gz','ax','ay','az']
    t = time.time()

    api.rate(options.rate)
    api.streamstart(True)
    api.StreamWD()

    data = np.zeros((numToRead,1+6*6))

    x = 0
    while x < numToRead:

        api.StreamWD()
        p = api.getIMUPacket()

        '''
        Reset the watchdog each time. No reason not to since the serial link to the
        unit is not used for anything except this.
        '''
        api.StreamWD()

        if p:

            '''
            Append the data to my potential Matlab array
            '''
            imudata = p.IMUData()
            if not imudata == None:
                #showPacket(p.pkID, imudata)
                imuidx = 0
                data[x][0] = p.pkID
                data[x][1:] = imudata.ravel()
            else:
                print ("None IMU Data returned")
        else:
            print "Bad Packet"
        x = x + 1
        if x % 100 == 0:
            print("Total Count:%d" % x)

    tdiff = time.time() - t

    if writeFile:

        if os.path.isfile(outfile):
            os.unlink(outfile)

        print "Saving data to %s" % outfile
        sp.io.savemat(outfile, mdict={
            'Data': data,
            'Name':options.dataname,
            'T':1.0/float(options.rate)})

    api.streamstop()

    print("Total time:%6.6f Time per Packet:%6.6f" % (tdiff,tdiff/x))

    print ("Done")
