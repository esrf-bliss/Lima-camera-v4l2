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
#ifndef V4L2VIDEOCTRLOBJ_H
#define V4L2VIDEOCTRLOBJ_H
#include "lima/HwVideoCtrlObj.h"
#include "lima/HwInterface.h"
#include "lima/Debug.h"
#include "lima/SizeUtils.h"
#include "lima/Constants.h"
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <set>

namespace lima
{
  namespace V4L2
  {
    class DetInfoCtrlObj;
    class VideoCtrlObj : public HwVideoCtrlObj
    {
      DEB_CLASS_NAMESPC(DebModCamera,"VideoCtrlObj","V4L2");
    public:
      VideoCtrlObj(int fd);
      virtual ~VideoCtrlObj();

      // Video interface
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
      
      virtual void setBin(const Bin&){};
      virtual void setRoi(const Roi&){};

     // --- Detector Info
      void getMaxImageSize(Size&);

      void getCurrImageType(ImageType&);
      void setCurrImageType(ImageType);
      
      void getDetectorModel(std::string& det_model);
      // --- Syn Obj
      void getMinMaxExpTime(double& min,double& max);
      void setExpTime(double  exp_time);
      void getExpTime(double& exp_time);
      
      void setNbHwFrames(int  nb_frames);
      void getNbHwFrames(int& nb_frames);

      // --- Acquisition interface
      void reset(HwInterface::ResetLevel reset_level);
      void prepareAcq();
      void startAcq();
      void stopAcq();
      void getStatus(HwInterface::StatusType& status);
      int getNbHwAcquiredFrames();
      int getV4l2Fd() { return m_fd; }
      
    private:
      class _AcqThread;
      friend class _AcqThread;
      void _unmap();
      void _map();

      std::string 		m_det_model;
      int 			m_fd;
      struct v4l2_buffer 	m_buffer;
      unsigned char* 		m_buffers[2];
      int 			m_nb_frames;
      int 			m_acq_frame_id;
      bool 			m_acq_started;
      bool			m_acq_thread_run;
      std::set<VideoMode>	m_available_format;
      _AcqThread*		m_acq_thread;
      int			m_pipes[2];
      bool			m_quit;
      Cond			m_cond;
      bool                      m_live;
      double                    m_gain;
   };
  }
}
#endif
