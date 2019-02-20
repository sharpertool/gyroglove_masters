#!/usr/bin/env python

import sys
import serial
import platform
import glob
import time
import re
import os.path
from IMUPacket import *
from optparse import OptionParser
import numpy as np
#import scipy as sp
#import scipy.io

class GloveAPI(object):

    def __init__(self):
        super(GloveAPI,self).__init__()
        self.ser = None

        self.pktrd = IMUPacketRead()

    def openPort(self,baud=115200,port=None,retries=20):
        if not port:
            port = self.getPorts()

        nTries = 0
        while nTries < retries:
            try:
                self.ser = serial.Serial(port,baud, timeout=1)
                if self.ser.isOpen():
                    print("Opened port %s at %d baud" % (port,baud))
                else:
                    print("Serial Port did not open..!")
                    raise("Bad thing happend.")
                #self.ctrl = ControllerAPI.ControllerComm(port,9600,timeout=1)
            except:
                raise "Could not open serial port!"
                self.ser = None
                return False

            retc = self.initHardware()
            if not retc:
                print("Init Failed. Reset Serial port")
                self.ser = None
                nTries = nTries + 1
            else:
                return True

    def initHardware(self):
        '''
        The first time we write to the hardware it appears that the FTDI chip resets
        it.. this has been problematic. It requires about 3 seconds for the hardware
        to come up, so I do a write, then a read with a long timeout, and see what happens.
        This should insure we are ready to run....
        '''
        self.ser.timeout = 3
        print("Timeout set to 3 seconds. Getting Time")
        retc = self.sendPacket(b'gettime\n')
        if not retc:
            retc = self.sendPacket(b'gettime\n')
        print("Timeout done. Restore to 1 second")
        self.ser.timeout = 1

        return retc

    def getPorts(self):
        if platform.system() == 'Darwin':
            """scan for available ports. return a list of device names."""
            ports = glob.glob('/dev/tty.usbserial-A400*')
            return ports[0]
        elif platform.system() == 'Windows':
            return "COM11" # No super good way to determine this..

    def sendPacket(self,packet):
        self.ser.write(packet)

        while (1):
            t = time.time()
            retval = self.ser.readline()
            try:
                tdiff = time.time() - t
                if tdiff > self.ser.timeout:
                    print("Timeout probably occured:%f" % tdiff)
                    return None

                retval = retval.decode()
                m = re.search("Ok|Fail",retval,re.IGNORECASE)
                if m:
                    print("{0} Result:{1}".format(packet.decode(),retval))
                    break
                else:
                    print("{0}".format(retval))
                    pass
            except:
                # Invalid format, cannot decode it..
                print("Exception")
                return None

        return retval

    def i2crd(self,port,ID, addr, nbytes = 1):
        packet = "i2crd {0} {1} {2} {3}\n".format(port, ID, addr, nbytes)
        retval = self.sendPacket(packet.encode('utf-8'))

        m = re.search("Ok:(.*)\r\n",retval)
        if m:
            dvals = m.group(1)
            dvals = dvals.strip()
            dlist = []
            dlist = dvals.split()
            vallist = []
            for d in dlist:
                try:
                    vallist.append(int(d,16))
                except:
                    pass
            return vallist

        return None

    def i2cwr(self,port,ID,addr,data):
        if isinstance(data,(list,tuple)):
            data = " ".join(data)

        packet = "i2cwr {0} {1} {2} {3}\n".format(port, ID, addr, data)
        retval = self.sendPacket(packet.encode('utf-8'))

        if retval:
            m = re.search("Ok:(.*)\r\n",retval)
            if m:
                return True

        return False

    def streamstart(self,bUseGyro = False):
        if bUseGyro:
            retval = self.sendPacket(b'streamstart 1\n')
        else:
            retval = self.sendPacket(b'streamstart 0\n')

        if retval:
            m = re.search("Ok",retval)
            if m:
                return True
        return False

    def StreamWD(self):
        self.ser.write(b'wd\n')
        #print("Reset watchdog")
        return True

    def streamstop(self):
        retval = self.sendPacket(b'streamstop\n')

        if retval:
            m = re.search("Ok",retval)
            if m:
                return True
        return False

    def rate(self,nHz):
        print("Setting rate to %d" % nHz)
        retval = self.sendPacket(b'rate %d\n' % nHz)

        if retval:
            m = re.search("Ok",retval)
            print("Retval from rate setting:%s" % retval)
            if m:
                return True

    def fiforeset(self):
        retval = self.sendPacket(b'fiforeset\n')

        if retval:
            m = re.search("Ok",retval)
            if m:
                return True
        return False

    def fifoenable(self):
        retval = self.sendPacket(b'fifoenable\n')

        if retval:
            m = re.search("Ok",retval)
            if m:
                return True
        return False

    def debug(self,bOn):
        if bOn:
            retval = self.sendPacket(b'debug 1\n')
        else:
            retval = self.sendPacket(b'debug 0\n')

        if retval:
            m = re.search("Ok",retval)
            if m:
                return True
        return False

    def reset(self):
        while (1):
            print ("Attempting Reset..")
            retval = self.sendPacket(b'reset\n')
            self.ser.write(b'checkids\n')

            bFail = False
            for x in range(0,12):
                retval = self.ser.readline()
                print ("Checkid returned:%s for %d" % (retval,x))
                try:
                    m = re.search("NAck",retval,re.IGNORECASE)
                    if m:
                        # One of these fails..
                        bFail = True
                except:
                    # Invalid format, cannot decode it..
                    bFail = True

            if not bFail:
                self.configimu()
                return True

    def startTimeToClockTime(self,startTime):
        minutes = startTime * 5
        hr = int(minutes / 60)
        min = minutes % 60
        if hr > 11:
            ampm = 'pm'
            hr = hr = 12
        else:
            ampm = 'am'

        clkTime = "{0:02d}:{1:02d}{2}".format(hr,min,ampm)
        return clkTime

    def getTime(self):

        retval = self.sendPacket(b'gettime\n')
        m = re.search("Ok:(\d+)",retval)
        if m:
            dt = m.group(1)
            try:
                unixTime = int(dt)
            except:
                print("Failed to convert datatime value:%s to integer" % dt)
                return None

            tm = time.localtime(unixTime)
            timeString = time.strftime("%b %d,%Y %H:%M",tm)

            return timeString
        else:
            return None

    def getBaud(self):

        retval = self.sendPacket(b'baudquery\n')
        if retval:
            m = re.search("Ok:(.*)",retval)
            if m:
                dt = m.group(1)
                print("Baud Values:%s" % dt)

        return None

    def setTime(self):
        timeval = time.time() - time.timezone
        packet = "settime   %d\n" % int(timeval)
        retval = self.sendPacket(packet.encode('utf-8'))
        if retval:
            if re.search("Fail",retval):
                return None
            return "Ok"
        else:
            return False

    def configimu(self):
        retval = self.sendPacket(b'configimu\n')

        if retval:
            m = re.search("Ok",retval)
            if m:
                return True
        return False


    def clearIMUPacketEngine(self):
        self.pktrd.Clear()

    def getIMUPacket(self):
        self.pktrd.Clear()
        t = time.time()
        while (self.pktrd.isValidPacket == False):
            self.pktrd.getChars(self.ser)
            tdiff = time.time() - t
            if tdiff > self.ser.timeout:
                print("Timeout occurred:%f" % tdiff)
                return None

        return self.pktrd.packet

def GetApi():
    return GloveAPI()

def showPacket(p,keys):
    #print("Packet: ID:%d Type:0x%x CRC:0x%x DataLen:%d" % (p.pkID, p.pkType, p.pkCRC, len(p.pkData)))
    imudata = p.Results()
    wd = int(p.WatchDog)
    strings = []
    for i in imudata:
        strings.append("\t".join(["%x" % i[k] for k in keys]))
    print("Wd:%d" % wd + " " + "\t".join(strings))

def printPacket(f,fkeys,p):
    imudata = p.Results()
    strings = []
    for i in imudata:
        strings.append("\t".join(["%d" % i[k] for k in fkeys]))
    f.write("\t".join(strings) + "\n")

def testIMU(api):
    #api.fiforeset()
    #api.rate(10)
    #api.configimu()
    #api.streamWD()
    #api.fifoenable()
    nToRead = 2
    num2Read = 2
    fiforeadcounter = 20
    for z in range (100):
        vals = api.i2crd(0,210,58,2)
        if vals:
            s = " ".join(["%8d" % x for x in vals])
            fifoSize = vals[0] << 8 or vals[1]
            print ("Fifo Size %d: " % (fifoSize) + s)

            if fifoSize > 4:
                fifoSize = fifoSize - 2
                nToRead = fifoSize - fifoSize % 2
                if nToRead > 20:
                    nToRead = 16

                if nToRead > 0:
                    print ("Reading %d from fifo\n" % nToRead)
                    fvals = api.i2crd(0,210,60,nToRead)
                    if fvals:
                        s = ",".join(["%8d" % x for x in fvals])
                        print("Fifo:" + s + "\n")
                    fiforeadcounter = fiforeadcounter - 1
                    if fiforeadcounter == 0:
                        print ("Done")
                        break

            if vals[0] == 2:
                pass
                #break
    api.fiforeset()

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

    print ("Got this far, established communication")

    if options.stop:
        api.streamstop()
        sys.exit(0)

    numToRead = options.count

    if options.outfile:
        writeFile = True
    else:
        writeFile = False

    api.clearIMUPacketEngine()
    keys = ['sentinal','temp','gx','gy','gz','ax','ay','az','footer']
    #keys = ['gx','gy','gz','ax','ay','az']
    #packets = []
    t = time.time()

    fkeys = ['temp','gx','gy','gz','ax','ay','az']

    api.rate(options.rate)
    api.streamstart(True)
    api.StreamWD()

    data = np.zeros((numToRead,1+6*7))

    nLeft2Stream = numToRead
    api.StreamWD()
    for x in range(0,numToRead):

        api.StreamWD()
        p = api.getIMUPacket()
        nLeft2Stream = nLeft2Stream - 1

        '''
        Reset the watchdog each time. No reason not to since the serial link to the
        unit is not used for anything except this.
        '''
        api.StreamWD()

        if p:
            #if writeFile:
                #printPacket(f,fkeys,p)
            #packets.append(p)
            showPacket(p,keys)

            '''
            Append the data to my potential Matlab array
            '''
            imudata = p.Results()
            imuidx = 0
            data[x][0] = p.pkID

            for i in imudata:
                #print("Adding IMU %d of %d\n" % (imuidx,x))
                colidx = 0
                for k in fkeys:
                    data[x][7*imuidx+colidx+1] = i[k]
                    colidx = colidx + 1
                imuidx = imuidx + 1
        else:
            print "Bad Packet"

    tdiff = time.time() - t

    if writeFile:

        if os.path.isfile(outfile):
            os.unlink(outfile)

        sp.io.savemat(outfile, mdict={
            'Data': data,
            'Name':options.dataname,
            'T':1.0/float(options.rate)})
        #f.close()

    api.streamstop()

    print("Total time:%6.6f Time per Packet:%6.6f" % (tdiff,tdiff/x))

    print ("Done")
