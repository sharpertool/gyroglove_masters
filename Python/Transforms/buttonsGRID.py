#dhochegger 2008
import direct.directbase.DirectStart
from direct.showbase import DirectObject
from direct.gui.DirectGui import *
from pandac.PandaModules import Point3,Vec3,Vec4
from direct.interval.LerpInterval import LerpFunc
from direct.interval.IntervalGlobal import *
import sys
count = 8
class buttons (DirectObject.DirectObject):
    def __init__ (self):
        XX = 0
        for x in range(count):
            XX += .1
            button = DirectButton(text = ("button"), command = self.button, scale = (.08, .05, .05),rolloverSound = "", relief = None)
            button.setColorScale(0,0,0,.6)
            button.setTransparency(1)
            button.setPos(-1.18, 0,  .85-XX)
    def button (self):
        print "..."
b = buttons() 