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
#include "V4L2Interface.h"
#include "V4L2DetInfoCtrlObj.h"
#include "V4L2SyncCtrlObj.h"
#include "V4L2Camera.h"
 
using namespace lima;
using namespace lima::V4L2;

class Interface::_Callback : public Camera::Callback
{
  DEB_CLASS_NAMESPC(DebModCamera, "Interface::_Callback", "V4L2");
public:
  _Callback(Interface& i) : m_interface(i) {}
  virtual bool newFrame(int frame_id,const unsigned char* srcPt)
  {
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(frame_id);

    StdBufferCbMgr& buffer_mgr = m_interface.m_buffer_ctrl_obj.getBuffer();
    void* framePt = buffer_mgr.getFrameBufferPtr(frame_id);
    const FrameDim& fDim = buffer_mgr.getFrameDim();
    memcpy(framePt,srcPt,fDim.getMemSize());
    HwFrameInfoType frame_info;
    frame_info.acq_frame_nb = frame_id;
    return buffer_mgr.newFrameReady(frame_info);
  }
private:
  Interface& m_interface;
};

Interface::Interface(const char* dev_path)
{
  DEB_CONSTRUCTOR();
  m_cbk = new _Callback(*this);
  m_cam = new Camera(m_cbk,dev_path);
  m_det_info = new DetInfoCtrlObj(*m_cam);
  m_sync = new SyncCtrlObj(*m_cam);
}

Interface::~Interface()
{
  DEB_DESTRUCTOR();
  
  delete m_sync;
  delete m_det_info;
  delete m_cam;
}

void Interface::getCapList(CapList &cap_list) const
{
  DEB_MEMBER_FUNCT();
  
  cap_list.push_back(HwCap(m_sync));
  cap_list.push_back(HwCap(m_det_info));
  cap_list.push_back(HwCap(&m_buffer_ctrl_obj));
}

void Interface::reset(ResetLevel reset_level)
{
  //Not managed
}

void Interface::prepareAcq()
{
  DEB_MEMBER_FUNCT();

  m_cam->prepareAcq();
}

void Interface::startAcq()
{
  DEB_MEMBER_FUNCT();
      
  StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();
  buffer_mgr.setStartTimestamp(Timestamp::now());

  m_cam->startAcq();
}

void Interface::stopAcq()
{
  DEB_MEMBER_FUNCT();

  m_cam->stopAcq();
}

void Interface::getStatus(StatusType &status)
{
  DEB_MEMBER_FUNCT();

  m_cam->getStatus(status);
  
  DEB_RETURN() << DEB_VAR1(status);
}

int Interface::getNbHwAcquiredFrames()
{
  DEB_MEMBER_FUNCT();
  
  int acq_frames = m_cam->getNbHwAcquiredFrames();
  
  DEB_RETURN() << DEB_VAR1(acq_frames);
  return acq_frames;
}
