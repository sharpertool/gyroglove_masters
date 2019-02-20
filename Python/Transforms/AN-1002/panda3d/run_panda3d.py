#
# @file    panda3d/run_panda3d.py
# @version 1.0
# @author  Luke Tokheim, luke@motionnode.com
#
# Apply real-time data from the MotionNode sensor
# to a model in a Panda3D scene (www.panda3d.org).
#
from direct.showbase.ShowBase import ShowBase
from direct.task import Task
from direct.actor.Actor import Actor

from pandac.PandaModules import * 
 
class MyApp(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
  
        # Load a model.
        self.model = self.loader.loadModel('model')
        self.model.reparentTo(self.render)
        self.model.setPos(0, -50, 0)

        # Connect to the VRPN tracker server.
        client = VrpnClient('127.0.0.1')

        # Create a tracker node.
        self.tracker0 = TrackerNode(client, 'Tracker0')
        base.dataRoot.node().addChild(self.tracker0)

        # Connect the tracker node to our model.
        t2n = Transform2SG('t2n')
        self.tracker0.addChild(t2n)
        t2n.setNode(self.model.node())

        # Move the camera to its initial position.
        base.camera.setPos(0, 50, 0)
        base.camera.lookAt(self.model)

        # Enable the mouse controls.
        mat = Mat4(base.camera.getMat())
        base.mouseInterfaceNode.setMat(mat)
        base.enableMouse()

app = MyApp()
app.run()
