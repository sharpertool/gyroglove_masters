#!/usr/bin/env python

import sys
import serial
import platform
import glob
import time
import re
import os.path
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from IMUPacket import *
from IMUManager import *
from GloveAPI import GloveAPI
from Ui_GloveServer import *
from threading import Thread,Lock
from socket import *
from struct import *

#HOST = '192.168.1.147'
HOST = '127.0.0.1'
PORT = 5120

class DataWorker(Thread):
    '''
    Worker thread for retrieving results from the IMU.
    This thread runs a loop that captures data from the IMU and
    processes it into packets. it then takes the important IMU
    results and passes it to a function on the data object. This
    function uses a lock object to protect access from multiple
    threads, and serves as the data transfer point between this thread
    and the display thread.

    An enhancement would be to have this tread JUST capture the data
    and then have another thread or more handle the processing of the
    data, even the basic conversion and extraction of the data. Normally
    this might not be a good idea, but since I have an 8 Core machine
    at home, this makes a ton of sense...
    '''

    def __init__(self,api,dataObj,rate):
        super(DataWorker,self).__init__()

        self.api = api
        self.dataObj = dataObj
        self.stopme = False
        self.rate = rate

    def run(self):
        '''
        Do the hard work..
        '''
        print("worker started..")
        api = GloveAPI()

        while not api.openPort():
            print("Open port failed.. retry")

        print("Clear Packet Engine")
        api.clearIMUPacketEngine()
        print("Set Rate")
        api.rate(self.rate)
        print("Start Stream")
        api.streamstart(True)
        print("Set WD")
        api.StreamWD()

        while self.stopme == False:

            packet = api.getIMUPacket()
            if packet:
                data = packet.MeasuredData()
                self.dataObj.newPacket(packet.numIMUs,packet.pkID, data)
                api.StreamWD()
            else:
                '''
                A Timeout occured, see if I can recover
                '''
                print("Empty data returned")
                api.streamstart(True)
                api.StreamWD()

        print("Worker asked to quit")

class SocketWorker(Thread):
    '''
    Worker thread for accepting socket connections.
    Will listen, then accept connections. While a connection
    is established, it will read data and then send it back.
    '''

    def __init__(self,parent):
        super(SocketWorker,self).__init__()

        self.parent  = parent

    def run(self):
        '''
        Do the hard work..
        '''


        print ("Setting up the socket")
        s = socket(AF_INET, SOCK_STREAM)
        s.bind((HOST,PORT))

        while True:
            print ("Waiting for a connection")
            s.listen(1)
            conn,addr = s.accept()
            print "Connected by ",addr

            packetCount = 0

            while 1:
                data = conn.recv(1024)
                if data:
                    if re.match("start",data):
                        self.parent.sStart()
                    elif re.match("stop",data):
                        self.parent.sStop()
                    elif re.match("data",data):
                        [id,d] = self.parent.sData()
                        if d:
                            packetCount = packetCount + 1
                            if packetCount % 100 == 0:
                                print("Total of %d packets sent." % packetCount)
                            num = conn.send(pack("BHH",0xb7,len(d),id)+d)
                            #print("Len d:%d number sent:%d" % (len(d), num))
                        else:
                            # Send a null packet, nothing left to send.
                            conn.send(pack("BHB", 0xb7,0,0))
                            #print("Sent null packet")
                    elif re.match("quit",data):
                        print("Exiting the socket connection")
                        conn.close()
                        return
                    else:
                        conn.send("Unknown command")
                else:
                    break

            conn.close()

class GloveServer(QMainWindow,
                       Ui_GloveServer):
    def __init__(self,parent=None):
        super(GloveServer, self).__init__(parent)

        self._running = False
        self.ser = None

        self.step = 0.01
        self.data = []
        self.pData = None
        self.numPackets = 0
        self.numIMUs = 0
        self.setupUi(self)
        #self.api = GloveAPI()
        #self.api.initHardware()
        self.editPackets.setText("0")
        self.editRate.setText("40")
        self.editNumIMUs.setText("0")

        self.btnStartStop.setText("Start")

        self.connect(self.btnStartStop, SIGNAL("clicked()"),
                     self.StartStop)

        self.connect(self.btnQuit, SIGNAL("clicked()"),
                     self.OnExit)

        self.lock = Lock()

        self.timer = QTimer(self)
        self.connect(self.timer,
                     SIGNAL("timeout()"),
                     self.onTimer)
        self.timer.start(1)

        self.sockworker = SocketWorker(self)
        self.sockworker.start()
        self.worker = None

    def OnExit(self):
        if self.worker:
            self.worker.stopme = True
            print("Stopped the worker")
        else:
            print("No worker to stop")
        s = socket(AF_INET, SOCK_STREAM)
        s.connect((HOST, PORT))
        s.send('quit')

    def newPacket(self,numIMUs,id,data):
        self.lock.acquire()
        self.numPackets = self.numPackets + 1
        self.numIMUs = numIMUs
        if len(self.data) == 10:
            """Shift the elements down, append new element to end """
            self.data = self.data[1:]
        self.data.append([id,data])
        self.lock.release()

    def sStart(self):
        self._Start()

    def sStop(self):
        self._Stop()

    def sData(self):
        self.lock.acquire()
        if self.data:
            [id,d] = self.data.pop(0)
        else:
            [id,d] = [0,None]
        self.lock.release()
        return [id,d]

    def _Start(self):
        self.lock.acquire()
        self.numPackets = 0
        self.data = []
        self.lock.release()

        if self._running == False:
            self._running = True

            self.btnStartStop.setText("Stop")
            '''
            The speed here is not critical, since we can process multiple
            values for each timer even. The timer locks the data, grabs all
            of the values present, then unlocks it.. running this loop faster
            would probably only serve to increase the overhead. It might make
            the viewer smoother, but I doubt it, since a 25ms update rate is
            faster than we can really discern anyway, assume we can discern a
            30Hz update rate...
            '''
            srate = self.editRate.text()
            try:
                rate = int(srate)
            except:
                rate = 100
                self.editRate.setText("%d" % rate)

            self.worker = DataWorker(None,self,rate)
            self.worker.start()

    def _Stop(self):
        if self._running:
            self.btnStartStop.setText("Start")
            self._running = False
            ''' Signal the worker thread to stop, then wait for it '''

            print("Telling worker to stop..")
            self.worker.stopme = True
            self.worker.join()
            print("Worker done")
            self.worker = None

    def StartStop(self):
        if self._running:
            self._Stop()
        else:
            self._Start()

    def onTimer(self):
        self.editPackets.setText("%d" % self.numPackets)
        self.editNumIMUs.setText("%d" % self.numIMUs)


if __name__ == "__main__":
    import sys

    app = QApplication(sys.argv)
    mw = GloveServer()
    mw.show()
    mw.raise_()
    app.exec_()
