#
# @file    vizard/run_vizard.py
# @version 1.0
# @author  Luke Tokheim, luke@motionnode.com
#
# Run a simple head tracking example with the MotionNode sensor
# using the Vizard Virtual Reality Toolkit (www.worldviz.com).
# Use the VRPN (www.cs.unc.edu/Research/vrpn/) device layer to
# read the tracking data.
#
# The sensor configuration happens in the file "vizsetupcfg.py".
# This simply starts up the tracker and shows a ground plane.
#
import viztracker

viztracker.go()

# Add a ground plane. This is part of the Vizard application resources.
viz.add('tut_ground.wrl')
