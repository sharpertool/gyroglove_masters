# grid v0.03 dhochegger 2008

from pandac.PandaModules import *
from direct.directtools.DirectGeometry import LineNodePath
import direct.directbase.DirectStart
from pandac.PandaModules import Point3,Vec3,Vec4
from direct.showbase import DirectObject
from pandac.PandaModules import NodePathCollection

global raws

import pivot
import gridFIELD
import buttonsGRID

raws1unit = 20
rawsHALFunit = 100

X1 = 10
X2 = -10
Y1 = 10
Y2 = -10


linesX = LineNodePath(render,'lines1',2,Vec4(.3,.3,.3,0))
linesXXX = LineNodePath(render,'lines1',.4,Vec4(.35,.35,.35,0))
axis = LineNodePath(render,'axis',4,Vec4(.2,.2,.2,0))
quad = LineNodePath(render,'quad',4,Vec4(.2,.2,.2,0))


x1 = (0,Y2,0)
x2 = (0,Y1,0)

x3 = (X2,0,0)
x4 = (X1,0,0)

global raws

axis.drawLines([[x1,x2],[x3,x4]])
axis.create()


q1 = (X1,Y1,0)
q2 = (X1,Y2,0)

q3 = (q2)
q4 = (X2,Y2,0)

q5 = (q4)
q6 = (X2,Y1,0)

q7= (q6)
q8 = (X1,Y1,0)

quad.drawLines([[q1,q2],[q3,q4],[q5,q6],[q7,q8]])
quad.create()
class gridPAINTER(DirectObject.DirectObject):
    global raws
    global linesXXX
    
    def __init__(self):


        d = 0
        for l in range (raws1unit-1):
            lO = len(gridFIELD.output)
            lO1 = lO - 1
            global field
            field1 = gridFIELD.output[lO1]
            field = float(field1)
            print field
            d+= field
            l1 = (X2+d,Y1,0)
            l2 = (X2+d,Y2,0)

            l3 = (X2,Y1-d,0)
            l4 = (X1,Y1-d,0)

            linesX.drawLines([[l1,l2],[l3,l4]])
        linesX.create()

        def getFIELD(task):
            #print gridFIELD.output
            return task.cont
        taskMgr.add(getFIELD,"getTask")

        global keyMap
        keyMap = {"ent":0,"clear":0}
        self.accept("enter", self.Keys, ["ent",1])
        self.accept("enter-up", self.Keys, ["ent",0])
        self.accept("space", self.Keys, ["clear",1])
        self.accept("space-up", self.Keys, ["clear",0])

    def Keys(self, key, value):
        keyMap[key] = value

        lO = len(gridFIELD.output)
        lO1 = lO - 1
        global field
        field1 = gridFIELD.output[lO1]
        field = float(field1)

        if field <= .1:
            global raws
            raws = field*2000
            field = .1

        if field > .2 and field < .4:
            global raws
            field = .2
            raws = field*500

        if field >= .4 and field <= .6:
            global raws
            field = .5
            raws = field*80

        if field >= .4 and field < 1:
            print "grid to big - it must be valued between 1 - .1"
            global raws
            field = .5
            raws = field*80

        if field == 1:
            print "grid is already on that state"
            global raws
            field = 0
            raws = 0

        if keyMap["ent"]:
            linesXXX.detachNode()
            global linesXXX
            linesXXX = LineNodePath(render,'lines1',.4,Vec4(.35,.35,.35,0))
            if field < .1:
                print "grid to small - it must be valued between 1 - .1"
            else:
                dd = 0
                for l in range (raws):
                    print field
                    dd+= field
                    l1 = (X2+dd,Y1,0)
                    l2 = (X2+dd,Y2,0)

                    l3 = (X2,Y1-dd,0)
                    l4 = (X1,Y1-dd,0)

                    linesXXX.drawLines([[l1,l2],[l3,l4]])
                linesXXX.create()

gp = gridPAINTER()
run()