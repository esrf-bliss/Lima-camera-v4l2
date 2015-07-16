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
#include <errno.h>
#include "V4L2DetInfoCtrlObj.h"
#include "V4L2VideoCtrlObj.h"

using namespace lima;
using namespace lima::V4L2;
DetInfoCtrlObj::DetInfoCtrlObj(VideoCtrlObj& video) : m_video(video)
{
  DEB_CONSTRUCTOR();

}

DetInfoCtrlObj::~DetInfoCtrlObj()
{
}

void DetInfoCtrlObj::getMaxImageSize(Size& max_image_size)
{
  m_video.getMaxImageSize(max_image_size);
}

void DetInfoCtrlObj::getDetectorImageSize(Size& det_image_size)
{
  m_video.getMaxImageSize(det_image_size);
}

void DetInfoCtrlObj::getDefImageType(ImageType& def_image_type)
{
  m_video.getCurrImageType(def_image_type);
}

void DetInfoCtrlObj::getCurrImageType(ImageType& curr_image_type)
{
  m_video.getCurrImageType(curr_image_type);
}

void DetInfoCtrlObj::setCurrImageType(ImageType  image_format)
{
  m_video.setCurrImageType(image_format);
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
  m_video.getDetectorModel(det_model);
}

void DetInfoCtrlObj::registerMaxImageSizeCallback(HwMaxImageSizeCallback& cb)
{
  m_mis_cb_gen.registerMaxImageSizeCallback(cb);
}

void DetInfoCtrlObj::unregisterMaxImageSizeCallback(HwMaxImageSizeCallback& cb)
{
  m_mis_cb_gen.unregisterMaxImageSizeCallback(cb);
}
