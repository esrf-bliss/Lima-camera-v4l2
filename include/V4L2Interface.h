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
#ifndef V4L2INTERFACE_H
#define V4L2INTERFACE_H
#include "lima/Debug.h"
#include "lima/HwInterface.h"

namespace lima
{
  namespace V4L2
  {
    class DetInfoCtrlObj;
    class SyncCtrlObj;
    class VideoCtrlObj;
    class Interface : public HwInterface
    {
      DEB_CLASS_NAMESPC(DebModCamera, "Interface", "V4L2");
    public:
      Interface(const std::string& dev_path = "/dev/video0");
      virtual ~Interface();

      virtual void getCapList(CapList &) const;
      
      virtual void reset(ResetLevel reset_level);
      virtual void prepareAcq();
      virtual void startAcq();
      virtual void stopAcq();
      virtual void getStatus(StatusType& status);
      
      virtual int getNbHwAcquiredFrames();
    private:
      int 			m_fd;
      DetInfoCtrlObj* 		m_det_info;
      SyncCtrlObj*		m_sync;
      CapList 	                m_cap_list;
      VideoCtrlObj*             m_video;
    };
  }
}
#endif
