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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <poll.h>
#include <unistd.h>

#include "V4L2Camera.h"
using namespace lima;
using namespace lima::V4L2;

class Camera::_AcqThread : public Thread
{
  DEB_CLASS_NAMESPC(DebModCamera, "Camera", "_AcqThread");
public:
  _AcqThread(Camera &aCam);

protected:
  virtual void threadFunction();

private:
  Camera& m_cam;
};

Camera::Camera(Camera::Callback* cbk,const char* video_device):
  m_cbk(cbk),
  m_fd(-1),
  m_nb_frames(1),
  m_acq_frame_id(-1),
  m_acq_started(false),
  m_quit(false)
{
  DEB_CONSTRUCTOR();

  memset(&m_buffer,0,sizeof(m_buffer));
  m_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  m_fd = v4l2_open(video_device,O_RDWR);
  if(m_fd < -1)
    THROW_HW_ERROR(Error) << "Error opening: " << video_device 
			  << "(" << strerror(errno) << ")";

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
  
  setCurrImageType(Bpp8);

  struct v4l2_streamparm streamparm;
  streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = v4l2_ioctl(m_fd,VIDIOC_G_PARM,&streamparm);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Error querying stream param : " << strerror(errno);

  if(streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
    {
      DEB_TRACE() << "Time per frame supported";
      struct v4l2_format format;
      format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
 
      v4l2_ioctl(m_fd,VIDIOC_G_FMT,&format);

      struct v4l2_frmivalenum frmivalenum;
      frmivalenum.width = format.fmt.pix.width;
      frmivalenum.height = format.fmt.pix.height;
      frmivalenum.pixel_format = format.fmt.pix.pixelformat;
      for(frmivalenum.index = 0;v4l2_ioctl(m_fd,VIDIOC_ENUM_FRAMEINTERVALS,&frmivalenum) != -1;
	  ++frmivalenum.index)
	{
	  if(frmivalenum.type == V4L2_FRMIVAL_TYPE_DISCRETE)
	    DEB_TRACE() << frmivalenum.discrete.numerator 
			<< "/" << frmivalenum.discrete.denominator;
	  else if(frmivalenum.type == V4L2_FRMIVAL_TYPE_STEPWISE)
	    {
	      DEB_TRACE() << "min : " 
			  << frmivalenum.stepwise.min.numerator << "/"
			  << frmivalenum.stepwise.min.denominator;
	      DEB_TRACE() << "max : " 
			  << frmivalenum.stepwise.max.numerator << "/"
			  << frmivalenum.stepwise.max.denominator;
	      DEB_TRACE() << "step : " 
			  << frmivalenum.stepwise.step.numerator << "/"
			  << frmivalenum.stepwise.step.denominator;
	    }
	  else
	    DEB_TRACE() << "Continuous";
	    
	}
    }
     
  

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE_AUTO;
  ctrl.value = V4L2_EXPOSURE_MANUAL;
  ret = v4l2_ioctl(m_fd,VIDIOC_S_CTRL,&ctrl);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't set exposure mode to manual" << strerror(errno);

  struct v4l2_requestbuffers requestbuff;
  requestbuff.count = sizeof(m_buffers) / sizeof(unsigned char*);
  requestbuff.type = m_buffer.type;
  requestbuff.memory = V4L2_MEMORY_MMAP;
  ret = v4l2_ioctl(m_fd,VIDIOC_REQBUFS,&requestbuff);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "req. buffers: " << strerror(errno);

  for (int i = 0;i < sizeof(m_buffers) / sizeof(unsigned char*);++i)
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

  if(pipe(m_pipes))
    THROW_HW_ERROR(Error) << "Can't open pipe";

  m_acq_thread = new _AcqThread(*this);
  m_acq_thread->start();
}

Camera::~Camera()
{
  DEB_DESTRUCTOR();

  AutoMutex aLock(m_cond.mutex());
  m_quit = true;
  m_cond.broadcast();
  close(m_pipes[1]);
  aLock.unlock();

  delete m_acq_thread;
  close(m_pipes[0]);

  for(int i = 0;i < sizeof(m_buffers) / sizeof(unsigned char*);++i)
    if(v4l2_munmap(m_buffers[i], m_buffer.length))
      DEB_ERROR() << "unmapping error: " << strerror(errno);

  v4l2_close(m_fd);
}

void Camera::getMaxImageSize(Size& max_size)
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

void Camera::getCurrImageType(ImageType& image_format)
{
}

void Camera::setCurrImageType(ImageType image_format)
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

void Camera::getDetectorModel(std::string& det_model)
{
  DEB_MEMBER_FUNCT();

  det_model = m_det_model;
  
  DEB_RETURN() << DEB_VAR1(det_model);
}
void Camera::getMinMaxExpTime(double& min,double& max)
{
  DEB_MEMBER_FUNCT();
  
  struct v4l2_queryctrl query;
  query.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_QUERYCTRL,&query);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get exposure time range " << strerror(errno);

  min = 1 / (query.maximum * 5.),max = 1 / (query.minimum * 5.);
  DEB_RETURN() << DEB_VAR2(min,max);
}
void Camera::getExpTime(double &exp_time)
{
  DEB_MEMBER_FUNCT();

 
  DEB_RETURN() << DEB_VAR1(exp_time);
}

void Camera::setExpTime(double exp_time)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(exp_time);

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  ctrl.value = 5 / exp_time ;
  int ret = v4l2_ioctl(m_fd,VIDIOC_S_CTRL,&ctrl);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't set exposure time" << strerror(errno);
}

void Camera::setNbHwFrames(int nb_frames)
{
  DEB_MEMBER_FUNCT();
  m_nb_frames = nb_frames;
}

void Camera::getNbHwFrames(int &nb_frames)
{
  DEB_MEMBER_FUNCT();
  nb_frames = m_nb_frames;
}

void Camera::reset(HwInterface::ResetLevel)
{
  DEB_MEMBER_FUNCT();

  THROW_HW_ERROR(NotSupported) << "Not implemented yet";
}

void Camera::prepareAcq()
{
  DEB_MEMBER_FUNCT();
  m_acq_frame_id = -1;

  for(int i = 0;
      i < sizeof(m_buffers) / sizeof(unsigned char*);++i)
    {
      m_buffer.index = i;
      //this will fail when calling prepareAcq a second time
      // maybe some fields in m_buffer have to be reset first.
      int ret = v4l2_ioctl(m_fd,VIDIOC_QBUF,&m_buffer);
      if(ret == -1)
	THROW_HW_ERROR(Error) << "Error queue buff " << strerror(errno);
      if(m_nb_frames && i > m_nb_frames) break;
    }
}

void Camera::startAcq()
{
  DEB_MEMBER_FUNCT();

  enum v4l2_buf_type buff_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(v4l2_ioctl(m_fd,VIDIOC_STREAMON,&buff_type) == -1)
    THROW_HW_ERROR(Error) << "Error starting stream : " << strerror(errno);

  AutoMutex aLock(m_cond.mutex());
  m_acq_started = true;
  m_cond.broadcast();
}

void Camera::stopAcq()
{
  DEB_MEMBER_FUNCT();

  AutoMutex aLock(m_cond.mutex());
  m_acq_started = false;
  write(m_pipes[1],"|",1);
  aLock.unlock();
}

void Camera::getStatus(HwInterface::StatusType& status)
{
  status.set(m_acq_thread_run ? HwInterface::StatusType::Exposure : HwInterface::StatusType::Ready);
}

int Camera::getNbHwAcquiredFrames()
{
  return m_acq_frame_id + 1;
}
// Acquisition thread

Camera::_AcqThread::_AcqThread(Camera& aCam) :
  m_cam(aCam)
{
  pthread_attr_setscope(&m_thread_attr,PTHREAD_SCOPE_PROCESS);
}

//---------------------------
//- Camera::_AcqThread::threadFunction()
//---------------------------
void Camera::_AcqThread::threadFunction()
{
  struct pollfd fds[2];
  fds[0].fd = m_cam.m_pipes[0];
  fds[0].events = POLLIN;
  fds[1].fd = m_cam.m_fd;
  fds[1].events = POLLIN;

  DEB_MEMBER_FUNCT();
  AutoMutex aLock(m_cam.m_cond.mutex());
  
  while(!m_cam.m_quit)
    {
      while(!m_cam.m_acq_started && !m_cam.m_quit)
	{
	  m_cam.m_acq_thread_run = false;
	  m_cam.m_cond.wait();
	}
      m_cam.m_acq_thread_run = true;
      if(m_cam.m_quit) return;

      bool continueAcq = true;

      while(continueAcq && 
	    (!m_cam.m_nb_frames || m_cam.m_acq_frame_id < (m_cam.m_nb_frames - 1)))
	{
	  aLock.unlock();
	  poll(fds,2,-1);

	  if(fds[0].revents)
	    {
	      char buffer[1024];
	      read(m_cam.m_pipes[0],buffer,sizeof(buffer));

	      aLock.lock();
	      continueAcq = m_cam.m_acq_started && !m_cam.m_quit;
	    }
	  else
	    {
	      int ret = v4l2_ioctl(m_cam.m_fd,VIDIOC_DQBUF,&m_cam.m_buffer);
	      if(ret == -1)
		{
		  DEB_ERROR() << "Error dequeue buff : " << strerror(errno);
		  continueAcq = false;
		}
	      else
		{
		  aLock.lock();
		  ++m_cam.m_acq_frame_id;
		  DEB_TRACE() << "Acq frame nb : " << m_cam.m_acq_frame_id;
		  if(m_cam.m_cbk)
		    continueAcq = m_cam.m_cbk->newFrame(m_cam.m_acq_frame_id,
							m_cam.m_buffers[m_cam.m_buffer.index]);
		  if(!m_cam.m_nb_frames ||
		     m_cam.m_acq_frame_id < (m_cam.m_nb_frames - sizeof(m_cam.m_buffers) / 
					     sizeof(unsigned char*)))
		    v4l2_ioctl(m_cam.m_fd,VIDIOC_QBUF,&m_cam.m_buffer);
		}
	    }
	}
      m_cam.m_acq_started = false;
      enum v4l2_buf_type buff_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if(v4l2_ioctl(m_cam.m_fd,VIDIOC_STREAMOFF,&buff_type) == -1)
	DEB_ERROR() << "Error stopping stream : " << strerror(errno);
    }
}
