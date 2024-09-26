//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2015
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <poll.h>
#include <unistd.h>
#include "V4L2DetInfoCtrlObj.h"
#include "V4L2VideoCtrlObj.h"

using namespace lima;
using namespace lima::V4L2;

///////////////
// Video Helper
///////////////

inline bool _from_v4l2_format_2_lima(int v4l_format,VideoMode &mode)
{
  bool found = true;
  switch(v4l_format)
    {
      /* RGB formats */
    case V4L2_PIX_FMT_RGB555:	mode = RGB555;		break;
    case V4L2_PIX_FMT_RGB565:	mode = RGB565;		break;
    case V4L2_PIX_FMT_BGR24:	mode = BGR24;		break;
    case V4L2_PIX_FMT_RGB24:	mode = RGB24;		break;
    case V4L2_PIX_FMT_BGR32:	mode = BGR32;		break;
    case V4L2_PIX_FMT_RGB32:	mode = RGB32;		break;
      /* Grey formats */
    case V4L2_PIX_FMT_GREY:	mode = Y8;		break;
    case V4L2_PIX_FMT_Y16:	mode = Y16;		break;

      /* Luminance+Chrominance formats */
    case V4L2_PIX_FMT_YUV422P:	mode = YUV422;		break;
    case V4L2_PIX_FMT_YUV411P:	mode = YUV411;		break;
    case V4L2_PIX_FMT_YUV420:	mode = I420;		break;
      /* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
    case V4L2_PIX_FMT_SBGGR8:	mode = BAYER_BG8;	break;
      //case V4L2_PIX_FMT_SGBRG8:
      //case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SBGGR16:	mode = BAYER_BG16;	break;

      /* compressed formats */
      // case V4L2_PIX_FMT_MJPEG: 	DEB_TRACE() << "As V4L2_PIX_FMT_MJPEG";break;
      // case V4L2_PIX_FMT_JPEG: 	DEB_TRACE() << "As V4L2_PIX_FMT_JPEG";break;
      // case V4L2_PIX_FMT_DV: 		DEB_TRACE() << "As V4L2_PIX_FMT_DV";break;
      // case V4L2_PIX_FMT_MPEG: 	DEB_TRACE() << "As V4L2_PIX_FMT_MPEG";break;
    default:
      found = false;
      break;
    }
  return found;
}

class VideoCtrlObj::_AcqThread : public Thread
{
  DEB_CLASS_NAMESPC(DebModCamera, "VideoCtrlObj", "_AcqThread");
public:
  _AcqThread(VideoCtrlObj &aVideo);

protected:
  virtual void threadFunction();

private:
  VideoCtrlObj& m_video;
};


VideoCtrlObj::VideoCtrlObj(int fd) : 
  m_fd(fd),
  m_nb_frames(1),
  m_acq_frame_id(-1),
  m_acq_started(false),
  m_acq_thread_run(false),
  m_quit(false),
  m_live(false),
  m_autoexp_supported(false),
  m_exptime_supported(false)
{
  DEB_CONSTRUCTOR();

  memset(&m_buffer,0,sizeof(m_buffer));
  m_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  for (unsigned int i = 0;i < sizeof(m_buffers) / sizeof(unsigned char*);++i)
    m_buffers[i] = NULL;

  struct v4l2_capability cap;
  int ret = v4l2_ioctl(m_fd, VIDIOC_QUERYCAP, &cap);

  if(ret == -1) 
    THROW_HW_ERROR(Error) << "Error querying cap: " << strerror(errno);

  DEB_TRACE() << DEB_VAR1(cap.driver) << ","
	      << DEB_VAR1(cap.card) << ","
	      << DEB_VAR1(cap.bus_info);

  m_det_model = (char*)cap.card;
  if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    THROW_HW_ERROR(Error) << "Error: dev. doesn't have VIDEO_CAPTURE cap.";

  struct v4l2_fmtdesc formatdesc;

  formatdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  for(formatdesc.index = 0;v4l2_ioctl(m_fd, VIDIOC_ENUM_FMT, &formatdesc) != -1;
      ++formatdesc.index)
    {
      VideoMode lima_video_mode;
      if(_from_v4l2_format_2_lima(formatdesc.pixelformat,lima_video_mode))
	m_available_format.insert(lima_video_mode);
      switch(formatdesc.pixelformat)
	{
	  /* RGB formats */
	case V4L2_PIX_FMT_RGB332: 	DEB_TRACE() << "As V4L2_PIX_FMT_RGB332";break;
	case V4L2_PIX_FMT_RGB444: 	DEB_TRACE() << "As V4L2_PIX_FMT_RGB444";break;
	case V4L2_PIX_FMT_RGB555: 	DEB_TRACE() << "As V4L2_PIX_FMT_RGB555";break;
	case V4L2_PIX_FMT_RGB565: 	DEB_TRACE() << "As V4L2_PIX_FMT_RGB565";break;
	case V4L2_PIX_FMT_RGB555X: 	DEB_TRACE() << "As V4L2_PIX_FMT_RGB555X";break;
	case V4L2_PIX_FMT_RGB565X: 	DEB_TRACE() << "As V4L2_PIX_FMT_RGB565X";break;
	case V4L2_PIX_FMT_BGR24: 	DEB_TRACE() << "As V4L2_PIX_FMT_BGR24";break;
	case V4L2_PIX_FMT_RGB24: 	DEB_TRACE() << "As V4L2_PIX_FMT_RGB24";break;
	case V4L2_PIX_FMT_BGR32: 	DEB_TRACE() << "As V4L2_PIX_FMT_BGR32";break;
	case V4L2_PIX_FMT_RGB32: 	DEB_TRACE() << "As V4L2_PIX_FMT_RGB32";break;

	  /* Grey formats */
	case V4L2_PIX_FMT_GREY: 	DEB_TRACE() << "As V4L2_PIX_FMT_GREY";break;
	case V4L2_PIX_FMT_Y16: 		DEB_TRACE() << "As V4L2_PIX_FMT_Y16";break;

	  /* Palette formats */
	case V4L2_PIX_FMT_PAL8: 	DEB_TRACE() << "As V4L2_PIX_FMT_PAL8";break;

	  /* Luminance+Chrominance formats */
	case V4L2_PIX_FMT_YVU410: 	DEB_TRACE() << "As V4L2_PIX_FMT_YVU410";break;
	case V4L2_PIX_FMT_YVU420: 	DEB_TRACE() << "As V4L2_PIX_FMT_YVU420";break;
	case V4L2_PIX_FMT_YUYV: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUYV";break;
	case V4L2_PIX_FMT_YYUV: 	DEB_TRACE() << "As V4L2_PIX_FMT_YYUV";break;
	case V4L2_PIX_FMT_YVYU: 	DEB_TRACE() << "As V4L2_PIX_FMT_YVYU";break;
	case V4L2_PIX_FMT_UYVY: 	DEB_TRACE() << "As V4L2_PIX_FMT_UYVY";break;
	case V4L2_PIX_FMT_VYUY: 	DEB_TRACE() << "As V4L2_PIX_FMT_VYUY";break;
	case V4L2_PIX_FMT_YUV422P: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUV422P";break;
	case V4L2_PIX_FMT_YUV411P: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUV411P";break;
	case V4L2_PIX_FMT_Y41P: 	DEB_TRACE() << "As V4L2_PIX_FMT_Y41P";break;
	case V4L2_PIX_FMT_YUV444: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUV444";break;
	case V4L2_PIX_FMT_YUV555: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUV555";break;
	case V4L2_PIX_FMT_YUV565: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUV565";break;
	case V4L2_PIX_FMT_YUV32: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUV32";break;
	case V4L2_PIX_FMT_YUV410: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUV410";break;
	case V4L2_PIX_FMT_YUV420: 	DEB_TRACE() << "As V4L2_PIX_FMT_YUV420";break;
	case V4L2_PIX_FMT_HI240: 	DEB_TRACE() << "As V4L2_PIX_FMT_HI240";break;
	case V4L2_PIX_FMT_HM12: 	DEB_TRACE() << "As V4L2_PIX_FMT_HM12";break;

	  /* two planes -- one Y, one Cr + Cb interleaved  */
	case V4L2_PIX_FMT_NV12: 	DEB_TRACE() << "As V4L2_PIX_FMT_NV12";break;
	case V4L2_PIX_FMT_NV21: 	DEB_TRACE() << "As V4L2_PIX_FMT_NV21";break;
	case V4L2_PIX_FMT_NV16: 	DEB_TRACE() << "As V4L2_PIX_FMT_NV16";break;
	case V4L2_PIX_FMT_NV61: 	DEB_TRACE() << "As V4L2_PIX_FMT_NV61";break;

	  /* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
	case V4L2_PIX_FMT_SBGGR8: 	DEB_TRACE() << "As V4L2_PIX_FMT_SBGGR8";break;
	case V4L2_PIX_FMT_SGBRG8: 	DEB_TRACE() << "As V4L2_PIX_FMT_SGBRG8";break;
	case V4L2_PIX_FMT_SGRBG8: 	DEB_TRACE() << "As V4L2_PIX_FMT_SGRBG8";break;
	case V4L2_PIX_FMT_SGRBG10: 	DEB_TRACE() << "As V4L2_PIX_FMT_SGRBG10";break;
	  /* 10bit raw bayer DPCM compressed to 8 bits */
	case V4L2_PIX_FMT_SGRBG10DPCM8: DEB_TRACE() << "As V4L2_PIX_FMT_SGRBG10DPCM8";break;
	  /*
	   * 10bit raw bayer, expanded to 16 bits
	   * xxxxrrrrrrrrrrxxxxgggggggggg xxxxggggggggggxxxxbbbbbbbbbb...
	   */
	case V4L2_PIX_FMT_SBGGR16: 	DEB_TRACE() << "As V4L2_PIX_FMT_SBGGR16";break;

	  /* compressed formats */
	case V4L2_PIX_FMT_MJPEG: 	DEB_TRACE() << "As V4L2_PIX_FMT_MJPEG";break;
	case V4L2_PIX_FMT_JPEG: 	DEB_TRACE() << "As V4L2_PIX_FMT_JPEG";break;
	case V4L2_PIX_FMT_DV: 		DEB_TRACE() << "As V4L2_PIX_FMT_DV";break;
	case V4L2_PIX_FMT_MPEG: 	DEB_TRACE() << "As V4L2_PIX_FMT_MPEG";break;

	  /*  Vendor-specific formats   */
	case V4L2_PIX_FMT_WNVA: 	DEB_TRACE() << "As V4L2_PIX_FMT_WNVA";break;
	case V4L2_PIX_FMT_SN9C10X: 	DEB_TRACE() << "As V4L2_PIX_FMT_SN9C10X";break;
	case V4L2_PIX_FMT_SN9C20X_I420: DEB_TRACE() << "As V4L2_PIX_FMT_SN9C20X_I420";break;
	case V4L2_PIX_FMT_PWC1: 	DEB_TRACE() << "As V4L2_PIX_FMT_PWC1";break;
	case V4L2_PIX_FMT_PWC2: 	DEB_TRACE() << "As V4L2_PIX_FMT_PWC2";break;
	case V4L2_PIX_FMT_ET61X251: 	DEB_TRACE() << "As V4L2_PIX_FMT_ET61X251";break;
	case V4L2_PIX_FMT_SPCA501: 	DEB_TRACE() << "As V4L2_PIX_FMT_SPCA501";break;
	case V4L2_PIX_FMT_SPCA505: 	DEB_TRACE() << "As V4L2_PIX_FMT_SPCA505";break;
	case V4L2_PIX_FMT_SPCA508: 	DEB_TRACE() << "As V4L2_PIX_FMT_SPCA508";break;
	case V4L2_PIX_FMT_SPCA561: 	DEB_TRACE() << "As V4L2_PIX_FMT_SPCA561";break;
	case V4L2_PIX_FMT_PAC207: 	DEB_TRACE() << "As V4L2_PIX_FMT_PAC207";break;
	case V4L2_PIX_FMT_MR97310A: 	DEB_TRACE() << "As V4L2_PIX_FMT_MR97310A";break;
	case V4L2_PIX_FMT_SQ905C: 	DEB_TRACE() << "As V4L2_PIX_FMT_SQ905C";break;
	case V4L2_PIX_FMT_PJPG: 	DEB_TRACE() << "As V4L2_PIX_FMT_PJPG";break;
	case V4L2_PIX_FMT_OV511: 	DEB_TRACE() << "As V4L2_PIX_FMT_OV511";break;
	case V4L2_PIX_FMT_OV518: 	DEB_TRACE() << "As V4L2_PIX_FMT_OV518";break;
	}
    }

  VideoMode PreferredVideoMode[] ={BAYER_BG8,BAYER_BG16,
				   I420,YUV411,YUV422,YUV444,
				   RGB555,RGB565,
				   RGB24,BGR24,
				   RGB32,BGR32,
				   Y64,Y32,Y16,Y8,};

  VideoMode start_video_mode;
  bool found_video_mode = false;
  for(unsigned i = 0;
      i < sizeof(PreferredVideoMode)/sizeof(VideoMode) && !found_video_mode;++i)
    {
      start_video_mode = PreferredVideoMode[i];
      found_video_mode = m_available_format.find(start_video_mode) != m_available_format.end();
   }

  if(!found_video_mode)
    THROW_HW_ERROR(Error) << "Can't find any video mode managed by lima core";
  
  setVideoMode(start_video_mode);
     
  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE_AUTO;
  ctrl.value = V4L2_EXPOSURE_AUTO;
  ret = v4l2_ioctl(m_fd,VIDIOC_S_CTRL,&ctrl);
  if(ret == -1) {
    m_autoexp_supported = false;
    DEB_WARNING() << "Auto Exposure NOT supported";
  } else {
    m_autoexp_supported = true;
  }
  if(pipe(m_pipes))
    THROW_HW_ERROR(Error) << "Can't open pipe";

  m_acq_thread = new _AcqThread(*this);
  m_acq_thread->start();
}


VideoCtrlObj::~VideoCtrlObj()
{
  DEB_DESTRUCTOR();

  AutoMutex aLock(m_cond.mutex());
  m_quit = true;
  m_cond.broadcast();
  close(m_pipes[1]);
  aLock.unlock();

  delete m_acq_thread;
  close(m_pipes[0]);

  for(unsigned i = 0;i < sizeof(m_buffers) / sizeof(unsigned char*);++i)
    if(v4l2_munmap(m_buffers[i], m_buffer.length))
      DEB_ERROR() << "unmapping error: " << strerror(errno);

  v4l2_close(m_fd);
}


// DetInfo
///////////

void VideoCtrlObj::getDetectorModel(std::string& det_model)
{
  DEB_MEMBER_FUNCT();

  det_model = m_det_model;
  
  DEB_RETURN() << DEB_VAR1(det_model);
}

void VideoCtrlObj::getMaxImageSize(Size& max_size)
{
  DEB_MEMBER_FUNCT();
  
  struct v4l2_format format;
  
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get the format: " << strerror(errno);

  max_size = Size(format.fmt.pix.width,format.fmt.pix.height);
  DEB_RETURN() << DEB_VAR1(max_size);
}

void VideoCtrlObj::getCurrImageType(ImageType& image_format)
{
  DEB_MEMBER_FUNCT();

  struct v4l2_format format;
  
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get the format: " << strerror(errno);
  
  switch(format.fmt.pix.pixelformat)
    {
    case V4L2_PIX_FMT_Y16:
    case V4L2_PIX_FMT_SBGGR16:
      image_format = Bpp16;break;
    default:
      image_format = Bpp8;break;
    }
  DEB_RETURN() << DEB_VAR1(image_format);
}

void VideoCtrlObj::setCurrImageType(ImageType image_format)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(image_format);

  VideoMode format;
  int ret = v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get the format: " << strerror(errno);

  bool found = false;
  for(std::set<VideoMode>::iterator i = m_available_format.begin();
      !found && i != m_available_format.end();++i)
    {
      switch(*i)
	{
	case Y16:
	case BAYER_RG16:
	case BAYER_BG16:
	  if(image_format == Bpp16)
	    {
	      found = true;
	      format = *i;
	    }
	  break;
	default:		// 8 bits
	  if(image_format == Bpp8)
	    {
	      found = true;
	      format = *i;
	    }
	  break;
	}
    }
  if(!found)
    THROW_HW_ERROR(NotSupported) << "Not supported by the camera";
  
  setVideoMode(format);
}

// Sync
////////

void VideoCtrlObj::getMinMaxExpTime(double& min,double& max)
{
  DEB_MEMBER_FUNCT();
  
  struct v4l2_queryctrl query;
  query.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_QUERYCTRL,&query);
  if(ret == -1) {
    DEB_WARNING() << "Exposure control not supported";
    m_exptime_supported = false;
    // open the range and ignore new settings
    min=0; max=1e6;
  } else {
    m_exptime_supported = true;
    min = 1 / (query.maximum * 5.),max = 1 / (query.minimum * 5.);
    DEB_RETURN() << DEB_VAR2(min,max);
  }    
}

void VideoCtrlObj::getExpTime(double &exp_time)
{
  DEB_MEMBER_FUNCT();

  if (m_exptime_supported) {
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    int ret = v4l2_ioctl(m_fd,VIDIOC_G_CTRL,&ctrl);
    if(ret == -1)
      THROW_HW_ERROR(Error) << "Can't get exposure time " << strerror(errno);
    
    exp_time = 1 / (ctrl.value * 5.); // Fixed me!!!
  } else {
    DEB_WARNING() << "Exposure control not supported, just ignore value !!";
  }
  DEB_RETURN() << DEB_VAR1(exp_time);
}

void VideoCtrlObj::setExpTime(double exp_time)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(exp_time);

  if (m_exptime_supported) {
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    ctrl.value = 5 / exp_time ;
    int ret = v4l2_ioctl(m_fd,VIDIOC_S_CTRL,&ctrl);
    if(ret == -1)
      THROW_HW_ERROR(Error) << "Can't set exposure time" << strerror(errno);
  } else {
    DEB_WARNING() << "Exposure control not supported, just ignore value !!";
  }

}

void VideoCtrlObj::setNbHwFrames(int nb_frames)
{
  DEB_MEMBER_FUNCT();
  m_nb_frames = nb_frames;
}

void VideoCtrlObj::getNbHwFrames(int &nb_frames)
{
  DEB_MEMBER_FUNCT();
  nb_frames = m_nb_frames;
}

void VideoCtrlObj::reset(HwInterface::ResetLevel)
{
  DEB_MEMBER_FUNCT();

  THROW_HW_ERROR(NotSupported) << "Not implemented yet";
}


// Acq
////////

void VideoCtrlObj::prepareAcq()
{
  DEB_MEMBER_FUNCT();
  m_acq_frame_id = -1;
  
  
  // If VIDIOC_QBUF called, stream must be set on/off before new buffer query
  // The only trick I find to make it works !!
  enum v4l2_buf_type buff_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(v4l2_ioctl(m_fd,VIDIOC_STREAMON,&buff_type) == -1)
    THROW_HW_ERROR(Error) << "Error starting stream : " << strerror(errno);
  
  if(v4l2_ioctl(m_fd,VIDIOC_STREAMOFF,&buff_type) == -1)
    DEB_ERROR() << "Error stopping stream : " << strerror(errno);
  for(unsigned i = 0;
      i < sizeof(m_buffers) / sizeof(unsigned char*);++i)
    {
      m_buffer.index = i;
      int ret = v4l2_ioctl(m_fd,VIDIOC_QBUF,&m_buffer);
      if(ret == -1)
	THROW_HW_ERROR(Error) << "Error queue buff " << strerror(errno);
      if(m_nb_frames && i > unsigned(m_nb_frames)) break;
    }
}

void VideoCtrlObj::startAcq()
{
  DEB_MEMBER_FUNCT();

  enum v4l2_buf_type buff_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(v4l2_ioctl(m_fd,VIDIOC_STREAMON,&buff_type) == -1)
    THROW_HW_ERROR(Error) << "Error starting stream : " << strerror(errno);

  AutoMutex aLock(m_cond.mutex());
  m_acq_started = true;
  m_cond.broadcast();

}

void VideoCtrlObj::stopAcq()
{
  DEB_MEMBER_FUNCT();

  AutoMutex aLock(m_cond.mutex());
  m_acq_started = false;
  write(m_pipes[1],"|",1);
  aLock.unlock();
}

void VideoCtrlObj::getStatus(HwInterface::StatusType& status)
{
  status.set(m_acq_thread_run ? HwInterface::StatusType::Exposure : HwInterface::StatusType::Ready);
}

int VideoCtrlObj::getNbHwAcquiredFrames()
{
  DEB_MEMBER_FUNCT();

  return m_acq_frame_id + 1;
}
// Acquisition thread
//////////////////////

VideoCtrlObj::_AcqThread::_AcqThread(VideoCtrlObj& aVideo) :
  m_video(aVideo)
{
  pthread_attr_setscope(&m_thread_attr,PTHREAD_SCOPE_PROCESS);
}


void VideoCtrlObj::getSupportedVideoMode(std::list<VideoMode>& aList) const
{
  DEB_MEMBER_FUNCT();

  for(std::set<VideoMode>::iterator i = m_available_format.begin();
      i != m_available_format.end();++i)
    aList.push_back(*i);
}


void VideoCtrlObj::setVideoMode(VideoMode mode)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(mode);

  struct v4l2_format format;
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get the format: " << strerror(errno);

  switch(mode)
    {
    case Y8:		format.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;		break;
    case Y16:		format.fmt.pix.pixelformat = V4L2_PIX_FMT_Y16;		break;

    case RGB555:	format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB555;	break;
    case RGB565:	format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;	break;
    case RGB24:		format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;	break;
    case RGB32:		format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;	break;
    case BGR24:		format.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;	break;
    case BGR32:		format.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;	break;

    case BAYER_BG8:	format.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR8;	break;
    case BAYER_BG16:	format.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR16;	break;

    case I420:		format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;	break;
    case YUV411:	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV411P;	break;
    case YUV422:	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV422P;	break;
    default:
      THROW_HW_ERROR(NotSupported) << "Not implemented yet!";
    }

  _unmap();
  ret = v4l2_ioctl(m_fd,VIDIOC_S_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't set the format: " << strerror(errno);
    
  //Setting the frame rate to the max
  struct v4l2_streamparm streamparm;
  streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = v4l2_ioctl(m_fd,VIDIOC_G_PARM,&streamparm);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Error querying stream param : " << strerror(errno);

  if(streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
  {
    DEB_ALWAYS() << "Time per frame supported";
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
 
    v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);

    struct v4l2_frmivalenum frmivalenum;
    frmivalenum.width = format.fmt.pix.width;
    frmivalenum.height = format.fmt.pix.height;
    frmivalenum.pixel_format = format.fmt.pix.pixelformat;
    __u32 denominator = 1;
    __u32 numerator = 1;
    for(frmivalenum.index = 0;
        v4l2_ioctl(m_fd,VIDIOC_ENUM_FRAMEINTERVALS,&frmivalenum) != -1;
        ++frmivalenum.index)
    {
      if(frmivalenum.type == V4L2_FRMIVAL_TYPE_DISCRETE)
      {
        DEB_TRACE() << frmivalenum.discrete.numerator 
                    << "/" << frmivalenum.discrete.denominator;
        if( 1.0*frmivalenum.discrete.denominator /
                        frmivalenum.discrete.numerator >
            1.0*denominator / numerator)
        {
            denominator = frmivalenum.discrete.denominator;
            numerator = frmivalenum.discrete.numerator;
        }
      }
      else
      {
          //Either V4L2_FRMIVAL_TYPE_STEPWISE or V4L2_FRMIVAL_TYPE_CONTINUOUS
          if(frmivalenum.type == V4L2_FRMIVAL_TYPE_STEPWISE)
            DEB_TRACE() << "Stepwise";
          else
            DEB_TRACE() << "Continuous";
          
          DEB_TRACE() << "min : " 
              << frmivalenum.stepwise.min.numerator << "/"
              << frmivalenum.stepwise.min.denominator;
          DEB_TRACE() << "max : " 
              << frmivalenum.stepwise.max.numerator << "/"
              << frmivalenum.stepwise.max.denominator;
          DEB_TRACE() << "step : " 
              << frmivalenum.stepwise.step.numerator << "/"
              << frmivalenum.stepwise.step.denominator;
              
          denominator = frmivalenum.stepwise.max.denominator;
          numerator = frmivalenum.stepwise.max.numerator;
          
          //We exit the loop now
          //since there is no index>0 in this case
          //(stepwise/continuous)
          break;
        }
        //else
          //DEB_TRACE() << "Continuous";
     
    }
    DEB_TRACE() << "Setting fps to " << 1.0*denominator / numerator <<
                    "(" << denominator << "/" << numerator << ")";
    streamparm.parm.capture.timeperframe.denominator=denominator;
    streamparm.parm.capture.timeperframe.numerator=numerator;
    ret = v4l2_ioctl(m_fd,VIDIOC_S_PARM,&streamparm);
    if(ret == -1)
      THROW_HW_ERROR(Error) << "Error setting frame rate : " << strerror(errno);
  } else {
    DEB_ALWAYS() << "Time per frame NOT supported";    
  }
    
  _map();
}

void VideoCtrlObj::getVideoMode(VideoMode& mode) const
{
  DEB_MEMBER_FUNCT();
  
  struct v4l2_format format;
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get the format: " << strerror(errno);
  
  _from_v4l2_format_2_lima(format.fmt.pix.pixelformat,mode);
  DEB_RETURN() << DEB_VAR1(mode);
}

void VideoCtrlObj::setLive(bool start_flag)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(start_flag);

  m_live = start_flag;
  if(start_flag)			// start
    {

      setNbHwFrames(0);
      
      // prepare to request buffers
      prepareAcq();


      enum v4l2_buf_type buff_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if(v4l2_ioctl(m_fd,VIDIOC_STREAMON,&buff_type) == -1)
	THROW_HW_ERROR(Error) << "Error starting stream : " << strerror(errno);

      AutoMutex aLock(m_cond.mutex());
      m_acq_started = true;
      m_cond.broadcast();
    }
  else
    {
      AutoMutex aLock(m_cond.mutex());
      m_acq_started = false;
      write(m_pipes[1],"|",1);
      aLock.unlock();
    }
}

void VideoCtrlObj::getLive(bool &flag) const
{
  DEB_MEMBER_FUNCT();
  flag = m_live;
}

void VideoCtrlObj::getGain(double &gain) const
{
  gain = m_gain;
}

void VideoCtrlObj::setGain(double gain)
{
  m_gain = gain;
}

void VideoCtrlObj::checkBin(Bin& bin)
{
  bin = Bin(1,1);		// Do not manage Hw Bin
}

void VideoCtrlObj::checkRoi(const Roi&, Roi& hw_roi)
{
  Size size;
  getMaxImageSize(size);
  hw_roi = Roi(0,0,size.getWidth(),size.getHeight()); // Do not manage Hw Roi
}

bool VideoCtrlObj::checkAutoGainMode(AutoGainMode mode) const
{
  return true;
}

void VideoCtrlObj::setHwAutoGainMode(AutoGainMode mode)
{
}

bool VideoCtrlObj::isAutoExposureSupported()
{
  DEB_MEMBER_FUNCT();
  return m_autoexp_supported;
}

void VideoCtrlObj::_unmap()
{
  DEB_MEMBER_FUNCT();
  if(!m_buffers[0]) return;			// nothing to free

  for(unsigned i = 0;i < sizeof(m_buffers) / sizeof(unsigned char*);++i)
    {
      if(m_buffers[i])
	{
	  if(v4l2_munmap(m_buffers[i], m_buffer.length))
	    DEB_ERROR() << "unmapping error: " << strerror(errno);
	  m_buffers[i] = NULL;
	}
    }

  struct v4l2_requestbuffers requestbuff;
  requestbuff.count = 0;
  requestbuff.type = m_buffer.type;
  requestbuff.memory = V4L2_MEMORY_MMAP;
  int ret = v4l2_ioctl(m_fd,VIDIOC_REQBUFS,&requestbuff);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "req. buffers: " << strerror(errno);
}

void VideoCtrlObj::_map()
{
  DEB_MEMBER_FUNCT();

  struct v4l2_requestbuffers requestbuff;
  requestbuff.count = sizeof(m_buffers) / sizeof(unsigned char*);
  requestbuff.type = m_buffer.type;
  requestbuff.memory = V4L2_MEMORY_MMAP;
  int ret = v4l2_ioctl(m_fd,VIDIOC_REQBUFS,&requestbuff);

  if(ret == -1)
    THROW_HW_ERROR(Error) << "req. buffers: " << strerror(errno);

  for (unsigned int i = 0;i < sizeof(m_buffers) / sizeof(unsigned char*);++i)
    {
      m_buffer.index = i;
      ret = v4l2_ioctl(m_fd, VIDIOC_QUERYBUF, &m_buffer);
      if (ret == -1) 
	THROW_HW_ERROR(Error) << "querying buffer " 
			      << i << ": " << strerror(errno);
      if (m_buffer.memory != V4L2_MEMORY_MMAP)
	THROW_HW_ERROR(Error)<< "memory type " 
			     << m_buffer.memory << ": not MMAP";
			
      int prot_flags = PROT_READ | PROT_WRITE;
      void *p = v4l2_mmap(NULL, m_buffer.length, prot_flags, 
			  MAP_SHARED, m_fd, m_buffer.m.offset);
      if (p == MAP_FAILED) 
	THROW_HW_ERROR(Error) << "mapping buffer " 
			      << i << ": " << strerror(errno);
      memset(p, 0, m_buffer.length);
      m_buffers[i] = (unsigned char *)p;
    }



}
//---------------------------
//- _AcqThread::threadFunction()
//---------------------------
void VideoCtrlObj::_AcqThread::threadFunction()
{
  struct pollfd fds[2];
  fds[0].fd = m_video.m_pipes[0];
  fds[0].events = POLLIN;
  fds[1].fd = m_video.m_fd;
  fds[1].events = POLLIN;

  DEB_MEMBER_FUNCT();
  AutoMutex aLock(m_video.m_cond.mutex());
  
  while(!m_video.m_quit)
    {
      while(!m_video.m_acq_started && !m_video.m_quit)
	{
	  m_video.m_acq_thread_run = false;
	  m_video.m_cond.wait();
	}
      m_video.m_acq_thread_run = true;
      if(m_video.m_quit) return;

      bool continueAcq = true;

      while(continueAcq && 
	    (!m_video.m_nb_frames || m_video.m_acq_frame_id < (m_video.m_nb_frames - 1)))
	{
	  aLock.unlock();
	  poll(fds,2,-1);

	  if(fds[0].revents)
	    {
	      char buffer[1024];
	      read(m_video.m_pipes[0],buffer,sizeof(buffer));

	      aLock.lock();
	      continueAcq = m_video.m_acq_started && !m_video.m_quit;
	    }
	  else
	    {
	      int ret = v4l2_ioctl(m_video.m_fd,VIDIOC_DQBUF,&m_video.m_buffer);
	      if(ret == -1)
		{
		  DEB_ERROR() << "Error dequeue buff : " << strerror(errno);
		  continueAcq = false;
		}
	      else
		{
		  aLock.lock();
		  ++m_video.m_acq_frame_id;
		  DEB_TRACE() << "Acq frame nb : " << m_video.m_acq_frame_id;
		  VideoMode mode;
		  Size size;
		  m_video.getVideoMode(mode);
		  m_video.getMaxImageSize(size);
		  continueAcq = m_video.callNewImage((char *)m_video.m_buffers[m_video.m_buffer.index],
						      size.getWidth(),
						      size.getHeight(),
						      mode);
		  if(!m_video.m_nb_frames ||
		     m_video.m_acq_frame_id < (m_video.m_nb_frames - sizeof(m_video.m_buffers) / 
					     sizeof(unsigned char*)))
		    v4l2_ioctl(m_video.m_fd,VIDIOC_QBUF,&m_video.m_buffer);
		}
	    }
	}
      m_video.m_acq_started = false;
      enum v4l2_buf_type buff_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if(v4l2_ioctl(m_video.m_fd,VIDIOC_STREAMOFF,&buff_type) == -1)
	DEB_ERROR() << "Error stopping stream : " << strerror(errno);
    }
}
