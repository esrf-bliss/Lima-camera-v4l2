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
#include <fcntl.h>
#include <errno.h>
#include "V4L2Interface.h"
#include "V4L2DetInfoCtrlObj.h"
#include "V4L2SyncCtrlObj.h"
#include "V4L2VideoCtrlObj.h"
#include "V4L2Camera.h"
 
using namespace lima;
using namespace lima::V4L2;

Interface::Interface(const char* dev_path)
{
  DEB_CONSTRUCTOR();

  m_fd = v4l2_open(dev_path,O_RDWR);
  if(m_fd < -1)
    THROW_HW_ERROR(Error) << "Error opening: " << dev_path 
			  << "(" << strerror(errno) << ")";

  m_video = new VideoCtrlObj(m_fd);
  m_det_info = new DetInfoCtrlObj(*m_video);
  m_sync = new SyncCtrlObj(*m_video);
  
  m_cap_list.push_back(HwCap(m_sync));
  m_cap_list.push_back(HwCap(m_det_info));
  m_cap_list.push_back(HwCap(m_video));
  m_cap_list.push_back(HwCap(&(m_video->getHwBufferCtrlObj())));
}

Interface::~Interface()
{
  DEB_DESTRUCTOR();
  
  delete m_sync;
  delete m_det_info;
  delete m_video;
}

void Interface::getCapList(CapList &cap_list) const
{
  DEB_MEMBER_FUNCT();
  cap_list = m_cap_list;
}

void Interface::reset(ResetLevel reset_level)
{
  //Not managed
}

void Interface::prepareAcq()
{
  DEB_MEMBER_FUNCT();
  m_video->prepareAcq();
}

void Interface::startAcq()
{
  DEB_MEMBER_FUNCT();
  
  m_video->getBuffer().setStartTimestamp(Timestamp::now());
  m_video->startAcq();
}

void Interface::stopAcq()
{
  DEB_MEMBER_FUNCT();

  m_video->stopAcq();
}

void Interface::getStatus(StatusType &status)
{
  DEB_MEMBER_FUNCT();

  m_video->getStatus(status);
  
  DEB_RETURN() << DEB_VAR1(status);
}

int Interface::getNbHwAcquiredFrames()
{
  DEB_MEMBER_FUNCT();
  
  int acq_frames = m_video->getNbHwAcquiredFrames();
  
  DEB_RETURN() << DEB_VAR1(acq_frames);
  return acq_frames;
}
