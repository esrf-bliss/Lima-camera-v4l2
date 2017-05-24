# CMakeLists.txt for V4l2 camera

SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--add-needed")

#SOURCEs
SET(v4l2_srcs src/V4L2Camera.cpp  src/V4L2Interface.cpp
	 src/V4L2DetInfoCtrlObj.cpp src/V4L2SyncCtrlObj.cpp
	 src/V4L2VideoCtrlObj.cpp)

FILE(STRINGS "VERSION" v4l2_vers)
ADD_LIBRARY(limav4l2 SHARED ${v4l2_srcs})

#INCLUDEs
TARGET_INCLUDE_DIRECTORIES(limav4l2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

#LIBRARIES
FIND_LIBRARY(V4L2_LIB v4l2)

TARGET_LINK_LIBRARIES(limav4l2 limacore)
TARGET_LINK_LIBRARIES(limav4l2 ${V4L2_LIB})


SET_TARGET_PROPERTIES(limav4l2 PROPERTIES VERSION ${v4l2_vers} SOVERSION "1")

INSTALL(TARGETS limav4l2 LIBRARY DESTINATION lib)

IF(COMPILE_SIP)
    SET(NAME "v4l2")
    SET(INCLUDES)
    FILE(GLOB sipfiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/sip" "${CMAKE_CURRENT_SOURCE_DIR}/sip/*.sip")
    FOREACH(sipfile ${sipfiles})
        SET(INCLUDES 
        "${INCLUDES} 
        %Include ${sipfile}")
    ENDFOREACH()

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/sip/limamodules.sip.in sip/limav4l2.sip)
    SET(SIP_CONCAT_PARTS 1)
    SET(SIP_INCLUDES ${SIP_INCLUDES}
        "${CMAKE_SOURCE_DIR}/third-party/Processlib/sip"
        "${CMAKE_BINARY_DIR}/sip/core"
        "${CMAKE_SOURCE_DIR}/third-party/Processlib/tasks/sip"
        "${CMAKE_SOURCE_DIR}/common/sip"
        "${CMAKE_SOURCE_DIR}/hardware/sip"
        "${CMAKE_SOURCE_DIR}/control/sip"
        "${CMAKE_SOURCE_DIR}/control/software_operation/sip"
        "${CMAKE_CURRENT_SOURCE_DIR}/sip")
    set(SIP_DISABLE_FEATURES WITH_CONFIG)
    set(SIP_CHECK_EXC ON)
    ADD_SIP_PYTHON_MODULE(limav4l2 ${CMAKE_CURRENT_BINARY_DIR}/sip/limav4l2.sip)
    TARGET_INCLUDE_DIRECTORIES(python_module_limav4l2 PRIVATE
                               ${PYTHON_INCLUDE_DIRS}
                               "${CMAKE_SOURCE_DIR}/sip"
                               "${CMAKE_SOURCE_DIR}/sip/core")
    TARGET_LINK_LIBRARIES(python_module_limav4l2 limav4l2)

    INSTALL(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/V4l2")
ENDIF()
