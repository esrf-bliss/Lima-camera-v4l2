###########################################################################
# This file is part of LImA, a Library for Image Acquisition
#
#  Copyright (C) : 2009-2017
#  European Synchrotron Radiation Facility
#  CS40220 38043 Grenoble Cedex 9 
#  FRANCE
# 
#  Contact: lima@esrf.fr
# 
#  This is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
# 
#  This software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--add-needed")

#SOURCEs
set(v4l2_srcs src/V4L2Camera.cpp  src/V4L2Interface.cpp
	 src/V4L2DetInfoCtrlObj.cpp src/V4L2SyncCtrlObj.cpp
	 src/V4L2VideoCtrlObj.cpp)

file(STRINGS "VERSION" v4l2_vers)
add_library(limav4l2 SHARED ${v4l2_srcs})

#INCLUDEs
target_include_directories(limav4l2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

#LIBRARIES
find_library(V4L2_LIB v4l2)
if("${V4L2_LIB}" STREQUAL "V4L2_LIB-NOTFOUND")
  message(FATAL_ERROR "v4l2 : libv4l2 not found, you can search it in cmake-gui or try to install it if it isn't here.")
endif()

target_link_libraries(limav4l2 limacore)
target_link_libraries(limav4l2 ${V4L2_LIB})


set_target_properties(limav4l2 PROPERTIES VERSION ${v4l2_vers} SOVERSION "1")

install(TARGETS limav4l2 LIBRARY DESTINATION lib)

if(COMPILE_SIP)
    set(NAME "v4l2")
    include(CameraRunSIP)
    install(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/V4l2")
endif()
