#!/usr/bin/env python

import re
import random
import time
import glob
import os.path
import socket
import shlex,subprocess

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.phonon import Phonon
from PyQt4 import Qt
import PyQt4.Qwt5 as Qwt
from PyQt4.Qwt5.anynumpy import *
from threading import Thread,Lock
import numpy as np
import scipy.io

import ui_imu_display

class IMU_Visualizer(QMainWindow,
                       ui_imu_display.Ui_IMU_Display):
    def __init__(self,parent=None):
        super(IMU_Visualizer, self).__init__(parent)

        self._running = False
        self.ser = None
        self.tval = 0

        #self.process = popen2("/bin/bash -x python ./wxVisualizer.py")
        #self.process = subprocess.Popen(shlex.split("/bin/bash -c \"python ./wxVisualizer.py\""))

        # Try the phonon thingy..

        self.mediasrc = Phonon.MediaObject()
        src = Phonon.MediaSource("../../DataCollection/GloveFlat_DI.mp4")
        self.mediasrc.setCurrentSource(src)

        self.setupUi(self)

        self.dataDir = '../../DataCollection'

        self.positionSlider.setMaximum(750)
        self.connect(self.positionSlider,
                     SIGNAL('sliderMoved(int)'),
                     self.setPosition)

        try:
            ''' Setup communication to the image server '''
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.client_socket.connect(("localhost", 5000))
        except:
            self.client_socket = None;

        self.LoadDataSets(self.comboData)
        self.connect(self.comboData,SIGNAL("currentIndexChanged(int)"), self.DataSetChanged)
        self.DataSetIndex = self.comboData.currentIndex()

        self.connect(self.buttonPlay,SIGNAL("clicked()"), self.Play)
        self.connect(self.buttonPause,SIGNAL("clicked()"), self.Pause)
        self.connect(self.buttonStop,SIGNAL("clicked()"), self.Stop)

        self.comboIMU.addItem('Hand')
        self.comboIMU.addItem('Middle')
        self.comboIMU.addItem('Thumb')
        self.comboIMU.addItem('Index')
        #self.comboIMU.addItem('Index')
        #self.comboIMU.addItem('Thumb')

        self.connect(self.comboIMU,SIGNAL("currentIndexChanged(int)"), self.IMUChanged)
        self.IMUIndex = self.comboIMU.currentIndex()

        self.intDataSelect = 0 # 0 = Gyros, 1 = Accelerometers

        self.size = 1800
        self.step = 0.00555 # 5ms.
        #self.t = arange(0.0,self.size*self.step,self.step)
        #self.x = zeros(self.size,Float)
        #self.y = zeros(self.size,Float)
        #self.z = zeros(self.size,Float)
        self.t = np.arange(0,self.size*self.step,self.step)
        self.x = np.zeros(self.size)
        self.y = np.zeros(self.size)
        self.z = np.zeros(self.size)

        self.nPos = 0

        self.pxAccel = Qwt.QwtPlot()
        self.vertPlots.addWidget(self.pxAccel)
        self.pyAccel = Qwt.QwtPlot()
        self.vertPlots.addWidget(self.pyAccel)
        self.pzAccel = Qwt.QwtPlot()
        self.vertPlots.addWidget(self.pzAccel)
        self.pxAccel.setTitle("X Accelerometer")
        self.pyAccel.setTitle("Y Accelerometer")
        self.pzAccel.setTitle("Z Accelerometer")

        self.pxGyro = Qwt.QwtPlot()
        self.gyroPlots.addWidget(self.pxGyro)
        self.pyGyro = Qwt.QwtPlot()
        self.gyroPlots.addWidget(self.pyGyro)
        self.pzGyro = Qwt.QwtPlot()
        self.gyroPlots.addWidget(self.pzGyro)
        self.pxGyro.setTitle("X Gyroscope")
        self.pyGyro.setTitle("Y Gyroscope")
        self.pzGyro.setTitle("Z Gyroscope")

        self.curveAX = Qwt.QwtPlotCurve("X Accel")
        self.curveAX.attach(self.pxAccel)
        self.curveAY = Qwt.QwtPlotCurve("Y Accel")
        self.curveAY.attach(self.pyAccel)
        self.curveAZ = Qwt.QwtPlotCurve("Z Accel")
        self.curveAZ.attach(self.pzAccel)

        self.curveAX.setPen(Qt.QPen(Qt.Qt.red))
        self.curveAY.setPen(Qt.QPen(Qt.Qt.blue))
        self.curveAZ.setPen(Qt.QPen(Qt.Qt.green))

        self.curveGX = Qwt.QwtPlotCurve("X Gyro")
        self.curveGX.attach(self.pxGyro)
        self.curveGY = Qwt.QwtPlotCurve("Y Gyro")
        self.curveGY.attach(self.pyGyro)
        self.curveGZ = Qwt.QwtPlotCurve("Z Gyro")
        self.curveGZ.attach(self.pzGyro)

        self.curveGX.setPen(Qt.QPen(Qt.Qt.red))
        self.curveGY.setPen(Qt.QPen(Qt.Qt.blue))
        self.curveGZ.setPen(Qt.QPen(Qt.Qt.green))

        self.DataSetChanged(self.DataSetIndex)

        self.lock = None
        self.data = []
        self.worker = None

        self.UpdateData(3)
        self.setupAllAxis()

    def setupAxis(self,axis):
        #axis.setAxisAutoScale(False)
        axis.setAxisScale(0,-32768, 32768)

    def setupAllAxis(self):
        #return
        self.setupAxis(self.pxGyro)
        self.setupAxis(self.pyGyro)
        self.setupAxis(self.pzGyro)
        self.setupAxis(self.pxAccel)
        self.setupAxis(self.pyAccel)
        self.setupAxis(self.pzAccel)

    def LoadDataSets(self,cb):
        matfiles = glob.glob(os.path.join(self.dataDir,'*.mat'))
        self.fileData = {}
        self.dataSetNames = []
        for f in matfiles:
            (drive,file) = os.path.split(f)
            (fname,ext) = os.path.splitext(file)
            m = re.match("(.*)(\d+)",fname)
            if m:
                field = m.group(1)
            else:
                field= fname

            self.fileData[fname] = {
                'Field' : field,
                'File'  : f,
                'Base'  : fname
            }

            vidFile = os.path.join(drive,fname + "_DI.mp4")
            if os.path.exists(vidFile):
                self.fileData[fname]['Video'] = vidFile

            cb.addItem(fname)
            self.dataSetNames.append(fname)
        cb.setCurrentIndex(2)

    def DataSetChanged(self,int):
        fname = self.dataSetNames[int]
        fdata = self.fileData[fname]

        file = fdata['File']
        mat = scipy.io.loadmat(file)
        fld = fdata['Field']
        self.mat = mat[fld]
        self.UpdateData(self.IMUIndex)

        if fdata.has_key('Video'):
            self.videoFile = fdata['Video']
            #self.Media = self.Instance.media_new(unicode(self.videoFile))
            #self.MediaPlayer.set_media(self.Media)
            #self.Media.parse()
            #self.MediaPlayer.play()
            if self.client_socket:
                self.client_socket.send("load \"%s\"" % self.videoFile)
        else:
            self.videoFile = None

        self.setupAllAxis()
        #self.comboData.setCurrentIndex(0)
        self.IMUIndex = self.comboIMU.currentIndex()
        self.UpdateData(self.IMUIndex)

    def Play(self):
        print("Starting playback")
        if self.client_socket:
            self.client_socket.send("play")

    def Pause(self):
        print("Pausing playback")
        if self.client_socket:
            self.client_socket.send("pause")

    def Stop(self):
        print("Stopping playback")
        if self.client_socket:
            self.client_socket.send("stop")

    def IMUChanged(self,int):
        self.IMUIndex = self.comboIMU.currentIndex()
        self.UpdateData(self.IMUIndex)

    def setPosition(self,pos):
        #print("Updating Position to %d" % pos)
        msPos = 1000*pos/60
        self.client_socket.send("pos %d" % msPos)

    def UpdateData(self,imunum):

        self.gx = self.mat[:,7*imunum+2]
        self.gy = self.mat[:,7*imunum+3]
        self.gz = self.mat[:,7*imunum+4]

        self.ax = self.mat[:,7*imunum+5]
        self.ay = self.mat[:,7*imunum+6]
        self.az = self.mat[:,7*imunum+7]

        self.curveAX.setData(self.t, self.ax)
        self.curveAY.setData(self.t, self.ay)
        self.curveAZ.setData(self.t, self.az)

        self.curveGX.setData(self.t, self.gx)
        self.curveGY.setData(self.t, self.gy)
        self.curveGZ.setData(self.t, self.gz)

        self.pxAccel.replot()
        self.pyAccel.replot()
        self.pzAccel.replot()

        self.pxGyro.replot()
        self.pyGyro.replot()
        self.pzGyro.replot()

    def AddData(self,v=[]):
        self.lock.acquire()
        self.data.append(v)
        self.lock.release()

    def timerEvent(self, e):
        if self._running == False:
            return

        self.lock.acquire()
        ldata = self.data
        self.data = []
        self.lock.release()
        #print("Updating with %d values" % len(ldata))
        if len(ldata) > 0:
            #self.StartStop()
            try:
                for v in ldata:
                    self.UpdateData(np.array(v))

            except:
                print ("Error...")
                #self.StartStop()

    def StartStop(self):
        if self._running:
            self.pbStartStop.setText("Start")
            self._running = False
            ''' Signal the worker thread to stop, then wait for it '''

            print("Telling worker to stop..")
            #self.worker.stopme = True
            #self.worker.join()
            print("Worker done")
            #self.worker = None
            #self.killTimer(self.timerid)

        else:
            self._running = True
            self.pbStartStop.setText("Stop")
            #self.api.streamOn(True)
            self.lock = Lock()
            '''
            The speed here is not critical, since we can process multiple
            values for each timer even. The timer locks the data, grabs all
            of the values present, then unlocks it.. running this loop faster
            would probably only serve to increase the overhead. It might make
            the viewer smoother, but I doubt it, since a 25ms update rate is
            faster than we can really discern anyway, assume we can discern a
            30Hz update rate...
            '''
            #self.timerid = self.startTimer(25)
            #self.worker = DataWorker(self.api,self,self.step)
            #self.worker.start()


if __name__ == "__main__":
    import sys

    app = QApplication(sys.argv)
    mw = IMU_Visualizer()
    mw.show()
    mw.raise_()
    app.exec_()
