.. _camera-v4l2:

V4l2 camera
--------------

.. image:: linuxtv.png

Intoduction
```````````
This new plugin aims to interface any v4l2 camera devices to LIMA framework. Some USB Webcams have been tested successfully.
Video for Linux 2 supports most of the market products, however you may encountered some limitations using Lima, please report
your problem and or your patch to lima@esrf.fr, we will be happy to improve this code for you.

Useful links :
  - http://linuxtv.org
  - http://en.wikipedia.org/wiki/Video4Linux


Installation & Module configuration
````````````````````````````````````
Depending or your linux flavor you may need to intall/update the v4l2 packages.

the package libv4l-dev is mandatory to compile the lima v4l2 plugin.

We recommend to install a useful tool **qv4l2** a Qt GUI. You can test your device and check supported video formats
and if the camera is supporting fixed exposure for instance.

-  follow the steps for the linux installation :ref:`linux_installation`
-  follow the steps for the windows installation :ref:`windows_installation`

The minimum configuration file is *config.inc* :

.. code-block:: sh

  COMPILE_CORE=1
  COMPILE_SIMULATOR=0
  COMPILE_SPS_IMAGE=1
  COMPILE_ESPIA=0
  COMPILE_FRELON=0
  COMPILE_MAXIPIX=0
  COMPILE_PILATUS=0
  COMPILE_V4L2=1
  COMPILE_CBF_SAVING=0
  export COMPILE_CORE COMPILE_SPS_IMAGE COMPILE_SIMULATOR \
         COMPILE_ESPIA COMPILE_FRELON COMPILE_MAXIPIX COMPILE_PILATUS \
         COMPILE_BASLER COMPILE_CBF_SAVING


-  start the compilation :ref:`linux_compilation`

-  finally for the Tango server installation :ref:`tango_installation`

Initialisation and Capabilities
````````````````````````````````

Camera initialisation
......................

The camera will be initialized  by creating V4l2::Camera object.  The Camera contructor
sets the camera with default parameters, and a device path is required (e.g. /dev/video0)

Std capabilites
................

This plugin has been implement in respect of the mandatory capabilites but with some limitations.
It is mainly a video controller (HwVideoCtrlObj) with the minimum set of of feature for the standard acquisition.
For instance the exposure control can not be available if the camera only support the auto-exposure mode.

* HwDetInfo
  
  getCurrImageType/getDefImageType(): it can change if the video mode change (see HwVideo capability).

  setCurrImageType(): It only supports Bpp8 and Bpp16.

* HwSync

  get/setTrigMode(): Only IntTrig mode is supported.
  
Optional capabilites
........................

The V4L2 camera plugin is a mostly a **Video** device which provides a limited interface for the acquisition(i.e, exposure, latency ..). 

* HwVideo

  The v4l2 cameras are pure video device we are supporting the commonly used formats:

 **Bayer formats**
   - BAYER_BG8
   - BAYER_BG16
 **Luminence+chrominance formats**  
   - YUV422
   - UYV411
   - YUV444
   - I420
 **RGB formats**
   - RGB555
   - RGB565
   - BGR24
   - RGB24
   - BGR32
   - RGB32
 **Monochrome formats**
    - Y8   
    - Y16
    - Y32
    - Y64 

  Use get/setVideoMode() on video object for video format. The lima plugin  will initialise the camera
  to a *preferred* video format by choosing one of the format the camera supports but through ordered
  list above.


Configuration
``````````````

  Simply plug your camera (USB device or other interface) on your computer, it should be automatically detected and
  a new device file is created like /dev/video0. The new device is maybe owned by **root:video**,so far an other user
  cannot access the device. In that case you should update /etc/group to add that user to the video group.

How to use
````````````
This is a python code example for a simple test:

.. code-block:: python

  from Lima import v4l2
  from lima import Core

 
  #------------------+
  # V4l2 device path |
  #                  v
  cam = v4l2.Camera('/dev/video0')


  hwint = v4l2.Interface(cam)
  ct = Core.CtControl(hwint)

  acq = ct.acquisition()


  # set and test video
  #

  video=ct.video()
  # to know which preferred format lima has selected
  print (video.getMode())
  video.startLive()
  video.stopLive()
  video_img = video.getLastImage()

  # set and test an acquisition
  #

  # setting new file parameters and autosaving mode
  saving=ct.saving()

  pars=saving.getParameters()
  pars.directory='/buffer/lcb18012/opisg/test_lima'
  pars.prefix='test1_'
  pars.suffix='.edf'
  pars.fileFormat=Core.CtSaving.TIFF
  pars.savingMode=Core.CtSaving.AutoFrame
  saving.setParameters(pars)

  # now ask for and 10 frames
  acq.setNbImages(10) 
  
  ct.prepareAcq()
  ct.startAcq()

  # wait for last image (#9) ready
  lastimg = ct.getStatus().ImageCounters.LastImageReady
  while lastimg !=9:
    time.sleep(1)
    lastimg = ct.getStatus().ImageCounters.LastImageReady
 
  # read the first image
  im0 = ct.ReadImage(0)


