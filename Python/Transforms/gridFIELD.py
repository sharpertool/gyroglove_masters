import direct.directbase.DirectStart
from direct.gui.OnscreenText import OnscreenText
from direct.gui.DirectGui import *
from pandac.PandaModules import *
from direct.directtools.DirectGeometry import LineNodePath
from direct.gui.OnscreenText import OnscreenText
import buttonsGRID



global output
output = [1]

textObject = OnscreenText(text = "gridUNITS", pos = (-1.23, .94), scale = 0.04,fg = (0,0,0,.8))
textObject.setTransparency(TransparencyAttrib.MAlpha)

line = LineNodePath(render2d,'line',2,Vec4(.3,.3,.3,0))
line.drawLines([[(-.76,0,1),(-.76,0,-1)]])
line.create()

def react(input):

    output.append(input)
    print output

def clear():
    gU.enterText('')

gU = DirectEntry(text = "",scale = .05,width = 5,command=react,rolloverSound = "",
initialText="GridUNIT->", numLines = 1,focus=0,focusInCommand=clear,
clickSound = "")
gU.setColorScale(.6,.7,.6,.4)
gU.setTransparency(TransparencyAttrib.MAlpha)

gU.setPos(-1.3,0,.86) 