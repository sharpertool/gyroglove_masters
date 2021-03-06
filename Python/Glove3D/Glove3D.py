# Importing math constants and functions
import os
import os.path
from socket import AF_INET, SOCK_DGRAM, socket

from direct.directtools.DirectGeometry import LineNodePath
from direct.gui.DirectGui import OnscreenText
from direct.showbase.ShowBase import ShowBase
from direct.task import Task
from panda3d.core import Vec4
from pandac.PandaModules import WindowProperties

modelDir = os.path.join(os.path.dirname(__file__), 'models')


class World(ShowBase):

    def __init__(self):
        super().__init__(self)

        # Initialize self variables
        self.environ = None
        self.bufsize = 2048
        self.addr = None
        self.my_socket = None
        self.z = None
        self.handmodel = None
        self.hand = None
        self.knuckle = None
        self.tknuckle = None
        self.thumb = None

        # This creates the on screen title that is in every tutorial
        self.title = OnscreenText(text="Gyro Glove",
                                  style=1, fg=(1, 1, 1, 1),
                                  pos=(0.87, -0.95), scale=.07)

        # Turn cursor on
        props = WindowProperties()
        props.setCursorHidden(False)
        self.win.requestProperties(props)

        self.setBackgroundColor(.6, .6, 1)  # Set the background color

        self.trackball.node().setPos(0, 30, -3)
        self.trackball.node().setHpr(-70, 0, 5)

        self.draw_grid()

        self.frameNav = self.draw_axis(self.render, [0, 0, 0])
        self.frameBody = self.draw_axis(self.render, [0, 0, 0])

        self.load_models()  # Load and position our models

        self.init_socket()
        self.taskMgr.add(self.roll_task, "rollTask")

    def load_environment(self):
        self.environ = self.loader.loadModel("models/environment")
        self.environ.reparentTo(self.render)
        self.environ.setScale(0.25, 0.25, 0.25)
        self.environ.setPos(-8, 42, 0)

    def draw_grid(self):
        raws1unit = 20
        rawsHALFunit = 100

        X1 = 10
        X2 = -10
        Y1 = 10
        Y2 = -10

        linesX = LineNodePath(self.render, 'lines1', 2, Vec4(.3, .3, .3, 0))
        linesXXX = LineNodePath(self.render, 'lines1', .4, Vec4(.35, .35, .35, 0))
        axis = LineNodePath(self.render, 'axis', 4, Vec4(.2, .2, .2, 0))
        quad = LineNodePath(self.render, 'quad', 4, Vec4(.2, .2, .2, 0))

        x1 = (0, Y2, 0)
        x2 = (0, Y1, 0)

        x3 = (X2, 0, 0)
        x4 = (X1, 0, 0)

        axis.drawLines([[x1, x2], [x3, x4]])
        axis.create()

        q1 = (X1, Y1, 0)
        q2 = (X1, Y2, 0)

        q3 = (q2)
        q4 = (X2, Y2, 0)

        q5 = (q4)
        q6 = (X2, Y1, 0)

        q7 = (q6)
        q8 = (X1, Y1, 0)

        quad.drawLines([[q1, q2], [q3, q4], [q5, q6], [q7, q8]])
        quad.create()

        gfOutput = [1]

        d = 0
        for l in range(raws1unit - 1):
            lO = len(gfOutput)
            lO1 = lO - 1
            global field
            field1 = gfOutput[lO1]
            field = float(field1)
            print(field)
            d += field
            l1 = (X2 + d, Y1, 0)
            l2 = (X2 + d, Y2, 0)

            l3 = (X2, Y1 - d, 0)
            l4 = (X1, Y1 - d, 0)

            linesX.drawLines([[l1, l2], [l3, l4]])
        linesX.create()

    def draw_axis(self, root, origin):
        PX = origin[0]
        PY = origin[1]
        PZ = origin[2]

        lengthX = PX + 1.5
        lengthY = PY + 1.5
        lengthZ = PZ + 1.5
        q1 = PX + .5
        q2 = -q1

        arrowLENGHT = PX + .2

        arrowXx1 = PY + .08
        arrowXx2 = PY - .08
        arrowXz2 = PX + 1.3

        arrowYx1 = PX + .08
        arrowYx2 = PX - .08
        arrowYz2 = PY + 1.3

        arrowZx1 = PX + .08
        arrowZx2 = PX - .08
        arrowZz2 = PZ + 1.3

        PIVarX = LineNodePath(root, 'pivotX', 3, Vec4(1, 0, 0, 1))
        PIVarY = LineNodePath(root, 'pivotY', 3, Vec4(0, 1, 1, 1))
        PIVarZ = LineNodePath(root, 'pivotZ', 3, Vec4(1, 1, 0, 1))

        PIVOThandler = LineNodePath(root, 'handler', 2, Vec4(1, 0, 1, 1))

        PIVarX.reparentTo(PIVOThandler)
        PIVarY.reparentTo(PIVOThandler)
        PIVarZ.reparentTo(PIVOThandler)

        arrowX1 = (lengthX, PY, PZ)
        arrowX2 = (arrowXz2, arrowXx1, PZ)
        arrowX3 = (arrowXz2, arrowXx2, PZ)

        arrowY1 = (PX, lengthY, PZ)
        arrowY2 = (arrowYx1, arrowYz2, PZ)
        arrowY3 = (arrowYx2, arrowYz2, PZ)

        arrowZ1 = (PX, PY, lengthZ)
        arrowZ2 = (arrowZx1, PY, arrowZz2)
        arrowZ3 = (arrowZx2, PY, arrowZz2)

        PIVarX.drawLines([[(PX, PY, PZ), (lengthX, PY, PZ)], [arrowX1, arrowX2], [arrowX1, arrowX3]])
        PIVarY.drawLines([[(PX, PY, PZ), (PX, lengthY, PZ)], [arrowY1, arrowY2], [arrowY1, arrowY3]])
        PIVarZ.drawLines([[(PX, PY, PZ), (PX, PY, lengthZ)], [arrowZ1, arrowZ2], [arrowZ1, arrowZ3]])

        PIVOThandler.drawLines([[(PX, PY, PZ), (PX + 0.5, PY, PZ)], [(PX + .5, PY, PZ), (PX, PY + .5, PZ)],
                                [(PX, PY + .5, PZ), (PX, PY, PZ)]])

        PIVarX.create()
        PIVarY.create()
        PIVarZ.create()
        PIVOThandler.create()

        return PIVOThandler

    def roll_task(self, task):

        data = self.read_data()
        while data:
            # self.x = data[0]
            # self.y = data[1]
            # self.hand.setX(self.x)
            # self.hand.setY(self.y)
            # print("Updated Position: %s" % ",".join([str(x) for x in data]))
            idx = int(data.pop(0))
            if idx == 0:
                self.hand.setPos(data[0], data[1], data[2])
                self.hand.setHpr(-data[5], data[3], -data[4])
                self.frameBody.setPos(data[0], data[1], data[2])
                self.frameBody.setHpr(-data[5], data[3], -data[4])
            elif idx < 5:
                self.fingers[idx - 1].setPos(data[0], data[1], data[2])
                self.fingers[idx - 1].setHpr(-data[5], data[3], -data[4])
            else:
                self.thumb.setPos(data[0], data[1], data[2])
                self.thumb.setHpr(-data[5], data[3], -data[4])

            data = self.read_data()

        return Task.cont

    def read_data(self):
        try:
            data, addr = self.my_socket.recvfrom(self.bufsize)
            data = [float(x) for x in data.split(',')]
            return data
        except:
            return None

    def init_socket(self):
        print("Starting the socket listener")
        host = "127.0.0.1"
        port = 5432
        self.bufsize = 2048
        self.addr = (host, port)
        self.my_socket = socket(AF_INET, SOCK_DGRAM)
        self.my_socket.settimeout(0.0)
        self.my_socket.bind(self.addr)

    def load_models(self):
        self.handmodel = self.loader.loadModel(os.path.join(modelDir, "HandBase.egg"))
        self.handmodel.setPosHpr(-2, -1.35, -0.3, 0, 0, 0)
        self.hand = self.render.attachNewNode('hand')
        self.handmodel.reparentTo(self.hand)

        # Create the knuckles
        self.knuckle = [self.hand.attachNewNode("knuckle" + str(i))
                        for i in range(4)]
        self.tknuckle = self.hand.attachNewNode("tknuckle")

        # Create the fingers and the thumb
        self.fingers = [self.knuckle[i].attachNewNode("finger" + str(i))
                        for i in range(4)]
        self.thumb = self.tknuckle.attachNewNode("thumb")

        # Set the position of the knuckles relative to the hand
        for i in range(4):
            self.knuckle[i].setPosHpr(2, 1 - float(i) * 2 / 3, 0, 0, 0, 0)
        self.tknuckle.setPosHpr(0, 1.4, 0, 0, 0, 0)

        # Load smileys to represent the knuckles
        self.smiley = [self.loader.loadModel("smiley.egg") for i in range(5)]
        [self.smiley[i].setScale(0.3) for i in range(5)]

        # Load models for the fingers and thumb
        self.models = [self.loader.loadModel(os.path.join(modelDir, "Fingertip.egg"))
                       for i in range(5)]

        [self.models[i].setPosHpr(0, 0.3, 0, 270, 0, 0) for i in range(5)]

        self.moves = [0 for i in range(4)]

        for i in range(4):
            # set the position and orientation of the ith panda node we just created
            # The Z value of the position will be the base height of the pandas.
            # The headings are multiplied by i to put each panda in its own position
            # around the carousel
            # self.fingers[i].setPosHpr(5, i*0.7+0.7, 0,270,-10,0)
            self.fingers[i].setPosHpr(2.5, 0, 0, 0, 0, 0)

            # Load the actual panda model, and parent it to its dummy node
            self.models[i].reparentTo(self.fingers[i])
            self.smiley[i].reparentTo(self.knuckle[i])

        self.thumb.setPosHpr(3, 1, -0.5, 20, -45, 0)
        self.models[4].reparentTo(self.thumb)
        self.smiley[4].reparentTo(self.tknuckle)

        # self.hand.flattenLight()

    def notify_pos(self, pos):
        print(f"I received a notification.. and tried to process it:{pos}")
        self.z = pos
        self.hand.set(pos)


w = World()
w.run()
