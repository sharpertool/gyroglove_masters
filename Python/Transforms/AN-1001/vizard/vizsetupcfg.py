#
# @file    vizard/vizsetupcfg.py
# @version 1.0
# @author  Luke Tokheim, luke@motionnode.com
#
# Vizard tracker configuration file for MotionNode
# based head tracking.
#
import viz

def createCustomComposite(id=0):
	viz.logNotice('MotionNode Head Tracking')
	
	# Use general VRPN tracker.
	vrpn = viz.add('vrpn7.dle')
	PPT_VRPN = 'Tracker0@localhost'
	head = vrpn.addTracker(PPT_VRPN)
	# Need to change rotation axes from MotionNode
	# to Vizard conventions.
	head.swapQuat([-3, -2, -1, 4])
	
	# Or, use the built in MotionNode tracker
	# support.
	#MotionNode = viz.add('MotionNode.dle')
	#head = MotionNode.addSensor()

	headfinal = viz.addGroup()
	headlink = viz.link (head, headfinal, enabled=False)
	import vizact
	vizact.onupdate (viz.PRIORITY_PLUGINS+1, headlink.update)
	headlink.postTrans([0, 0.1, 0]) # Apply 10 cm translate in Y axis

	import vizuniverse as VU
	comp = VU.VUCompositeTrackers()
	comp.storeTracker(comp.HEAD, headfinal)

	comp.createAvatarNone()
	comp.defineViewpoint()
	comp.finishTrackers()
	return comp