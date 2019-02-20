#pivot 3d axis dhochegger 2008

from pandac.PandaModules import *
from direct.directtools.DirectGeometry import LineNodePath
import direct.directbase.DirectStart
from pandac.PandaModules import Point3,Vec3,Vec4
#set the Pivot trans in X
global PX
PX = 10
#set the Pivot trans in Y
global PY
PY = 1
#set the Pivot trans in Z
global PZ
PZ = 1.5
global length
lengthX = PX + 1.5
lengthY = PY + 1.5
lengthZ = PZ + 1.5
global q1
q1 = PX + .5
global q2
q2 = -q1

arrowLENGHT = PX +.2

arrowXx1 = PY + .08
arrowXx2 = PY - .08
arrowXz2 = PX + 1.3

arrowYx1 = PX + .08
arrowYx2 = PX - .08
arrowYz2 = PY + 1.3

arrowZx1 = PX + .08
arrowZx2 = PX - .08
arrowZz2 = PZ + 1.3


PIVarX = LineNodePath(render,'pivotX',3,Vec4(1,0,0,1))
PIVarY = LineNodePath(render,'pivotY',3,Vec4(0,1,1,1))
PIVarZ = LineNodePath(render,'pivotZ',3,Vec4(1,1,0,1))

PIVOThandler = LineNodePath(render,'handler',2,Vec4(1,0,1,1))


arrowX1 = (lengthX,PY,PZ)
arrowX2 = (arrowXz2,arrowXx1,PZ)
arrowX3 = (arrowXz2,arrowXx2,PZ)

arrowY1 = (PX,lengthY,PZ)
arrowY2 = (arrowYx1,arrowYz2,PZ)
arrowY3 = (arrowYx2,arrowYz2,PZ)

arrowZ1 = (PX,PY,lengthZ)
arrowZ2 = (arrowZx1,PY,arrowZz2)
arrowZ3 = (arrowZx2,PY,arrowZz2)

PIVarX.drawLines([[(PX,PY,PZ),(lengthX,PY,PZ)],[arrowX1,arrowX2],[arrowX1,arrowX3]])
PIVarY.drawLines([[(PX,PY,PZ),(PX,lengthY,PZ)],[arrowY1,arrowY2],[arrowY1,arrowY3]])
PIVarZ.drawLines([[(PX,PY,PZ),(PX,PY,lengthZ)],[arrowZ1,arrowZ2],[arrowZ1,arrowZ3]])

PIVOThandler.drawLines([[(PX,PY,PZ),(PX+0.5,PY,PZ)],[(PX+.5,PY,PZ),(PX,PY+.5,PZ)],[(PX,PY+.5,PZ),(PX,PY,PZ)]])



PIVarX.create()
PIVarY.create()
PIVarZ.create()
PIVOThandler.create()
