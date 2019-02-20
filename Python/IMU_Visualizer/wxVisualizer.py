# experiment with wxPython's
# wx.media.MediaCtrl(parent, id, pos, size, style, backend)
# the backend (szBackend) is usually figured by the control
# wxMEDIABACKEND_DIRECTSHOW   Windows
# wxMEDIABACKEND_QUICKTIME    Mac OS X
# wxMEDIABACKEND_GSTREAMER    Linux (?)
# plays files with extension .mid .mp3 .wav .au .avi .mpg
# tested with Python24 and wxPython26 on Windows XP   vegaseat  10mar2006

import time
from threading import *
import wx
import wx.media
import os
import socket
import re

# Button Defintions
ID_START    = wx.NewId()
IS_STOP     = wx.NewId()

EVT_RESULT_ID   = wx.NewId()
EVT_POS_ID      = wx.NewId()
EVT_CONTROL_ID  = wx.NewId()
EVT_LOAD_ID     = wx.NewId()

def EVT_RESULT(win,func):
    '''Define Result Event.'''
    win.Connect(-1,-1,EVT_RESULT_ID, func)

class ResultEvent(wx.PyEvent):
    """Simple event to carry arbitrary result data."""
    def __init__(self, data):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_RESULT_ID)
        self.data = data

class PositionEvent(wx.PyEvent):
    """Update the position of the video"""
    def __init__(self, data):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_POS_ID)
        self.data = data

class LoadEvent(wx.PyEvent):
    """Update the position of the video"""
    def __init__(self, data):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_LOAD_ID)
        self.data = data

class ControlEvent(wx.PyEvent):
    """Play, Pause or Stop the video"""
    def __init__(self, data):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_CONTROL_ID)
        self.data = data

# Thread class that executes processing
class SocketListener(Thread):
    """Worker Thread Class."""
    def __init__(self, notify_window):
        """Init Worker Thread Class."""
        Thread.__init__(self)
        self._notify_window = notify_window
        # This starts the thread running on creation, but you could
        # also make the GUI thread responsible for calling this
        self.start()

    def run(self):
        """Run Worker Thread."""
        # This is the code executing in the new thread. Simulation of
        # a long process (well, 10s here) as a simple loop - you will
        # need to structure your processing so that you periodically
        # peek at the abort variable
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind(("", 5000))
        server_socket.listen(5)

        print ("Client Socket Setup")

        while 1:
            client_socket, address = server_socket.accept()
            print "I got a connection from ", address
            data = client_socket.recv(512)
            while data:
                cmdFound = False
                m = re.search("pos (\d+)",data,re.IGNORECASE)
                if m:
                    pos = int(m.group(1))
                    wx.PostEvent(self._notify_window,PositionEvent(pos) )
                    #print ("Updated Position to %d" % pos)
                    cmdFound = True
                m = re.search("load \"(.*)\"", data, re.IGNORECASE)
                if m:
                    # Re-load a new file
                    wx.PostEvent(self._notify_window,LoadEvent(m.group(1)))
                    cmdFound = True

                if re.search("play",data,re.IGNORECASE):
                    wx.PostEvent(self._notify_window,ControlEvent(2) )
                    print("Play Video")
                    cmdFound = True
                elif re.search("pause",data,re.IGNORECASE):
                    wx.PostEvent(self._notify_window,ControlEvent(1) )
                    print("Pause Video")
                    cmdFound = True
                elif re.search("stop",data,re.IGNORECASE):
                    wx.PostEvent(self._notify_window,ControlEvent(0) )
                    print("Stop Video")
                    cmdFound = True

                if not cmdFound:
                    print("Unknown Command:%s" % data)

                data = client_socket.recv(512)

class Panel1(wx.Panel):
    def __init__(self, parent, id):
        #self.log = log
        wx.Panel.__init__(self, parent, -1, style=wx.TAB_TRAVERSAL|wx.CLIP_CHILDREN)

        # Create some controls
        try:
            self.mc = wx.media.MediaCtrl(self, style=wx.SIMPLE_BORDER)
        except NotImplementedError:
            self.Destroy()
            raise

        loadButton = wx.Button(self, -1, "Load File")
        self.Bind(wx.EVT_BUTTON, self.onLoadFile, loadButton)

        playButton = wx.Button(self, -1, "Play")
        self.Bind(wx.EVT_BUTTON, self.onPlay, playButton)

        pauseButton = wx.Button(self, -1, "Pause")
        self.Bind(wx.EVT_BUTTON, self.onPause, pauseButton)

        stopButton = wx.Button(self, -1, "Stop")
        self.Bind(wx.EVT_BUTTON, self.onStop, stopButton)

        slider = wx.Slider(self, -1, 0, 0, 0, size=wx.Size(300, -1))
        self.slider = slider
        self.Bind(wx.EVT_SLIDER, self.onSeek, slider)

        self.st_file = wx.StaticText(self, -1, ".mid .mp3 .wav .au .avi .mpg", size=(200,-1))
        self.st_size = wx.StaticText(self, -1, size=(100,-1))
        self.st_len  = wx.StaticText(self, -1, size=(100,-1))
        self.st_pos  = wx.StaticText(self, -1, size=(100,-1))

        # setup the button/label layout using a sizer
        sizer = wx.GridBagSizer(5,5)
        sizer.Add(loadButton, (1,1))
        sizer.Add(playButton, (2,1))
        sizer.Add(pauseButton, (3,1))
        sizer.Add(stopButton, (4,1))
        sizer.Add(self.st_file, (1, 2))
        sizer.Add(self.st_size, (2, 2))
        sizer.Add(self.st_len,  (3, 2))
        sizer.Add(self.st_pos,  (4, 2))
        sizer.Add(self.mc, (5,1), span=(5,1))  # for .avi .mpg video files
        self.SetSizer(sizer)

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.onTimer)
        self.timer.Start(100)
        self.doLoadFile("../../DataCollection/IndexPointing_DI.mp4")

        self.Connect(-1,-1,EVT_RESULT_ID,self.OnResult)
        self.Connect(-1,-1,EVT_POS_ID, self.OnPosition)
        self.Connect(-1,-1,EVT_CONTROL_ID, self.OnControl)
        self.Connect(-1,-1,EVT_LOAD_ID, self.OnLoad)

        self.worker = SocketListener(self)

    def OnResult(self,event):
        if event.data is None:
            self.status.SetLabel("Starting Coputation")
        else:
            self.status.SetLabel('Result: %s' % event.data)

    def OnPosition(self,event):
        pos = event.data
        self.mc.Seek(pos)

    def OnControl(self,event):
        cmd = event.data
        if cmd == 1:
            self.onPause(0);
        if cmd == 2:
            self.onPlay(0)
        if cmd == 0:
            self.onStop(0)

    def OnLoad(self,event):
        file = event.data
        self.doLoadFile(file)

    def onLoadFile(self, evt):
        self.doLoadFile("../../DataCollection/IndexPointing_DI.mp4")
        #dlg = wx.FileDialog(self, message="Choose a media file",
        #                    defaultDir=os.getcwd(), defaultFile="",
        #                    style=wx.OPEN | wx.CHANGE_DIR )
        #if dlg.ShowModal() == wx.ID_OK:
        #    path = dlg.GetPath()
        #    self.doLoadFile(path)
        #dlg.Destroy()

    def doLoadFile(self, path):

        if not self.mc.Load(path):
            wx.MessageBox("Unable to load %s: Unsupported format?" % path, "ERROR", wx.ICON_ERROR | wx.OK)
        else:
            folder, filename = os.path.split(path)
            self.st_file.SetLabel('%s' % filename)
            self.mc.SetBestFittingSize()
            self.GetSizer().Layout()
            l = self.mc.Length()
            nFrames = l*60/1000.0
            print("Total Length:%f. Calculated frames:%d" % (l,nFrames))
            self.slider.SetRange(0, 10*self.mc.Length())
            self.slider.SetLineSize(int(10000/180))
            self.slider.SetPageSize(int(10000/180))
            #self.mc.Play()

    def onPlay(self, evt):
        self.mc.Play()

    def onPause(self, evt):
        self.mc.Pause()

    def onStop(self, evt):
        self.mc.Stop()

    def onSeek(self, evt):
        offset = self.slider.GetValue()
        self.mc.Seek(offset/10)

    def onTimer(self, evt):
        offset = self.mc.Tell()
        self.slider.SetValue(offset*10)
        self.st_size.SetLabel('size: %s ms' % self.mc.Length())
        self.st_len.SetLabel('( %d seconds )' % (self.mc.Length()/1000))
        self.st_pos.SetLabel('position: %d ms' % offset)


app = wx.PySimpleApp()
# create a window/frame, no parent, -1 is default ID
frame = wx.Frame(None, -1, "Show GyroGlove video files", size = (820, 650))
# call the derived class
Panel1(frame, -1)
frame.Show(1)
app.MainLoop()