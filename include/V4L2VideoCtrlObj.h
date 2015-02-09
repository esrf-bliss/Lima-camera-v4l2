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
#ifndef V4L2VIDEOCTRLOBJ_H
#define V4L2VIDEOCTRLOBJ_H
#include "lima/HwVideoCtrlObj.h"

namespace lima
{
  namespace V4L2
  {
    class DetInfoCtrlObj;
    class VideoCtrlObj : public HwVideoCtrlObj
    {
      DEB_CLASS_NAMESPC(DebModCamera,"VideoCtrlObj","V4L2");
    public:
      VideoCtrlObj(int fd,DetInfoCtrlObj&);
      virtual ~VideoCtrlObj();

      virtual void getSupportedVideoMode(std::list<VideoMode>& list) const;
      virtual void setVideoMode(VideoMode);
      virtual void getVideoMode(VideoMode&) const;

      virtual void setLive(bool);
      virtual void getLive(bool&) const;
      
      virtual void getGain(double&) const;
      virtual void setGain(double);
      virtual bool checkAutoGainMode(AutoGainMode) const;
      virtual void setHwAutoGainMode(AutoGainMode);

      virtual void checkBin(Bin& bin);
      virtual void checkRoi(const Roi& set_roi,Roi& hw_roi);
      
      virtual void setBin(const Bin&);
      virtual void setRoi(const Roi&);
      
    private:
      int		m_fd;
      DetInfoCtrlObj&	m_det_info;
      
      Cond		m_cond;
      bool		m_acq_started;
      int		m_pipes[2];
   };
  }
}
#endif
