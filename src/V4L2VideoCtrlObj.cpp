//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
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
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <errno.h>
#include "V4L2VideoCtrlObj.h"
#include "V4L2DetInfoCtrlObj.h"

using namespace lima;
using namespace lima::V4L2;

VideoCtrlObj::VideoCtrlObj(int fd,DetInfoCtrlObj& det_info) : 
  m_fd(fd),
  m_det_info(det_info)
{
}

VideoCtrlObj::~VideoCtrlObj()
{
}

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

void VideoCtrlObj::getSupportedVideoMode(std::list<VideoMode>& list) const
{
  std::list<int> formats;
  m_det_info.getAvailableFormat(formats);
  for(std::list<int>::iterator i = formats.begin();i != formats.end();++i)
    {
      VideoMode mode;
      if(_from_v4l2_format_2_lima(*i,mode)) list.push_back(mode);
    }
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

  ret = v4l2_ioctl(m_fd,VIDIOC_S_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't set the format: " << strerror(errno);
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

  if(start_flag)			// start
    {
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
