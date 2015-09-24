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
//#include <cmath>
#include "V4L2SyncCtrlObj.h"
#include "V4L2VideoCtrlObj.h"

using namespace lima;
using namespace lima::V4L2;

SyncCtrlObj::SyncCtrlObj(VideoCtrlObj& video) : 
  m_video(video),
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

void SyncCtrlObj::setExpTime(double exp_time)
{
  DEB_MEMBER_FUNCT();
  m_video.setExpTime(exp_time);
}

void SyncCtrlObj::getExpTime(double& exp_time)
{
  DEB_MEMBER_FUNCT();
  m_video.getExpTime(exp_time);
}

bool SyncCtrlObj::checkAutoExposureMode(AutoExposureMode mode) const
{
  DEB_MEMBER_FUNCT();
  
  
  bool check_flag = mode == HwSyncCtrlObj::ON;
  if (!m_video.isAutoExposureSupported() && mode == HwSyncCtrlObj::ON)
    return false;
  else return true;
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
  m_video.setNbHwFrames(nb_frames);
}

void SyncCtrlObj::getNbHwFrames(int& nb_frames)
{
  DEB_MEMBER_FUNCT();

//  nb_frames = m_nb_frames;
  m_video.getNbHwFrames(nb_frames);

  DEB_RETURN() << DEB_VAR1(nb_frames);
}

void SyncCtrlObj::getValidRanges(ValidRangesType& valid_ranges)
{
  DEB_MEMBER_FUNCT();

  valid_ranges.min_lat_time = 0.;
  valid_ranges.max_lat_time = 0.;

  m_video.getMinMaxExpTime(valid_ranges.min_exp_time, valid_ranges.max_exp_time);

  DEB_RETURN() << DEB_VAR1(valid_ranges);
}

