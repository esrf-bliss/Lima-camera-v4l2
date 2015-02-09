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
#include <errno.h>
#include "V4L2DetInfoCtrlObj.h"
#include "V4L2Interface.h"
#include "V4L2Camera.h"

using namespace lima;
using namespace lima::V4L2;
DetInfoCtrlObj::DetInfoCtrlObj(int fd) : m_fd(fd)
{
  DEB_CONSTRUCTOR();

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
      m_available_format.push_back(formatdesc.pixelformat);
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
}

DetInfoCtrlObj::~DetInfoCtrlObj()
{
}

void DetInfoCtrlObj::getMaxImageSize(Size& max_image_size)
{
  _maxImageSize(max_image_size);
}

void DetInfoCtrlObj::getDetectorImageSize(Size& det_image_size)
{
  _maxImageSize(det_image_size);
}

void DetInfoCtrlObj::getDefImageType(ImageType& def_image_type)
{
  _getCurrentImageType(def_image_type);
}

void DetInfoCtrlObj::getCurrImageType(ImageType& curr_image_type)
{
  _getCurrentImageType(curr_image_type);
}

void DetInfoCtrlObj::setCurrImageType(ImageType  image_format)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(image_format);

  struct v4l2_format format;
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get the format: " << strerror(errno);

  bool found = false;
  for(std::list<int>::iterator i = m_available_format.begin();
      !found && i != m_available_format.end();++i)
    {
      switch(*i)
	{
	case V4L2_PIX_FMT_GREY:
	case V4L2_PIX_FMT_YUV420:
	case V4L2_PIX_FMT_YVU420:
	  if(image_format == Bpp8)
	    {
	      found = true;
	      format.fmt.pix.pixelformat = *i;
	    }
	  break;
	case V4L2_PIX_FMT_Y16:
	  if(image_format == Bpp16)
	    {
	      found = true;
	      format.fmt.pix.pixelformat = *i;
	    }
	  break;
	}
    }
  if(!found)
    THROW_HW_ERROR(NotSupported) << "Not supported by the camera";
  
  ret = v4l2_ioctl(m_fd,VIDIOC_S_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't set the format: " << strerror(errno);
}

void DetInfoCtrlObj::getPixelSize(double& x_size,double &y_size)
{
  x_size = y_size = -1;		// Don't know
}

void DetInfoCtrlObj::getDetectorType(std::string& det_type)
{
  det_type = "V4L2";
}

void DetInfoCtrlObj::getDetectorModel(std::string& det_model)
{
  DEB_MEMBER_FUNCT();
  
  det_model = m_det_model;

  DEB_RETURN() << DEB_VAR1(det_model);
}

void DetInfoCtrlObj::registerMaxImageSizeCallback(HwMaxImageSizeCallback& cb)
{
  m_mis_cb_gen.registerMaxImageSizeCallback(cb);
}

void DetInfoCtrlObj::unregisterMaxImageSizeCallback(HwMaxImageSizeCallback& cb)
{
  m_mis_cb_gen.unregisterMaxImageSizeCallback(cb);
}

void DetInfoCtrlObj::_maxImageSize(Size &max_size)
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

void DetInfoCtrlObj::_getCurrentImageType(ImageType& image_format)
{
  DEB_MEMBER_FUNCT();

  struct v4l2_format format;
  
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get the format: " << strerror(errno);
  
  switch(format.fmt.pix.pixelformat)
    {
    case V4L2_PIX_FMT_GREY:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420:
      image_format = Bpp8;break;
    case V4L2_PIX_FMT_Y16:
      image_format = Bpp16;break;
    default:
      THROW_HW_ERROR(NotSupported) << "Not yet managed";
    }
  DEB_RETURN() << DEB_VAR1(image_format);
}
