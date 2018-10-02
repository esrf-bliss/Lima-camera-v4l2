set(V4L2_INCLUDE_DIRS)
set(V4L2_LIBRARIES)
set(V4L2_DEFINITIONS)

find_path(V4L2_INCLUDE_DIRS "libv4l2.h")
find_library(V4L2_LIBRARIES v4l2)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(V4L2 DEFAULT_MSG
  V4L2_LIBRARIES
  V4L2_INCLUDE_DIRS
)
