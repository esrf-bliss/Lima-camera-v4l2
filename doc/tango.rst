V4l2 Tango device
==================

This is the reference documentation of the V4l2 Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`V4l2 camera plugin <camera-v4l2>` section.

Properties
----------
================= =============== =============== =========================================================================
Property name	  Mandatory	  Default value	  Description
================= =============== =============== =========================================================================
video_device	  No		  /dev/video0	  The video device path
================= =============== =============== =========================================================================


Attributes
----------
This device has no attribute.

Commands
--------

=======================	=============== =======================	===========================================
Command name		Arg. in		Arg. out		Description
=======================	=============== =======================	===========================================
Init			DevVoid 	DevVoid			Do not use
State			DevVoid		DevLong			Return the device state
Status			DevVoid		DevString		Return the device state as a string
getAttrStringValueList	DevString:	DevVarStringArray:	Return the authorized string value list for
			Attribute name	String value list	a given attribute name
=======================	=============== =======================	===========================================

