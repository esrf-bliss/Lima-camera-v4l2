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
#include <cmath>
#include <errno.h>
#include "V4L2SyncCtrlObj.h"
#include "V4L2Camera.h"

using namespace lima;
using namespace lima::V4L2;

SyncCtrlObj::SyncCtrlObj(int fd) : 
  m_fd(fd),
  m_nb_frames(1)
{
}

SyncCtrlObj::~SyncCtrlObj()
{
}

bool SyncCtrlObj::checkTrigMode(TrigMode trig_mode)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(trig_mode);

  bool returnValue = trig_mode == IntTrig;

  DEB_RETURN() << DEB_VAR1(returnValue);

  return returnValue;
}

void SyncCtrlObj::setTrigMode(TrigMode trig_mode)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(trig_mode);

  if(!checkTrigMode(trig_mode))
    THROW_HW_ERROR(InvalidValue) << DEB_VAR1(trig_mode) << " is not supported";
}

void SyncCtrlObj::getTrigMode(TrigMode& trig_mode)
{
  DEB_MEMBER_FUNCT();

  trig_mode = IntTrig;

  DEB_RETURN() << DEB_VAR1(trig_mode);
}

void SyncCtrlObj::setExpTime(double)
{
  DEB_MEMBER_FUNCT();
  THROW_HW_ERROR(NotSupported) << "Should never been called?!?";
}

void SyncCtrlObj::getExpTime(double& exp_time)
{
  DEB_MEMBER_FUNCT();
  
  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_G_CTRL,&ctrl);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get exposure time " << strerror(errno);
  
  exp_time = 1 / (ctrl.value * 5.); // Fixed me!!!

  DEB_RETURN() << DEB_VAR1(exp_time);
}

bool SyncCtrlObj::checkAutoExposureMode(AutoExposureMode mode) const
{
  DEB_MEMBER_FUNCT();
  
  bool check_flag = mode == HwSyncCtrlObj::ON;
  
  DEB_RETURN() << DEB_VAR1(check_flag);
  return check_flag;
}

void SyncCtrlObj::setLatTime(double lat_time)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(lat_time);
  // Not managed
}

void SyncCtrlObj::getLatTime(double& lat_time)
{
  DEB_MEMBER_FUNCT();

  lat_time = 0.;

  DEB_RETURN() << DEB_VAR1(lat_time);
}

void SyncCtrlObj::setNbHwFrames(int nb_frames)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(nb_frames);
  m_nb_frames = nb_frames;
}

void SyncCtrlObj::getNbHwFrames(int& nb_frames)
{
  DEB_MEMBER_FUNCT();

  nb_frames = m_nb_frames;

  DEB_RETURN() << DEB_VAR1(nb_frames);
}

void SyncCtrlObj::getValidRanges(ValidRangesType& valid_ranges)
{
  DEB_MEMBER_FUNCT();

  valid_ranges.min_lat_time = 0.;
  valid_ranges.max_lat_time = 0.;

  struct v4l2_queryctrl query;
  query.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  int ret = v4l2_ioctl(m_fd,VIDIOC_QUERYCTRL,&query);
  if(ret == -1)
    THROW_HW_ERROR(Error) << "Can't get exposure time range " << strerror(errno);

  valid_ranges.min_exp_time = 1 / (query.maximum * 5.);
  valid_ranges.max_exp_time = 1 / (query.minimum * 5.);

  DEB_RETURN() << DEB_VAR1(valid_ranges);
}

