set(OPENNI2_ROOT "/usr/local" CACHE PATH "Root directory of OpenNI2")

find_path(OPENNI2_INCLUDE_DIR OpenNI.h HINTS "${OPENNI2_ROOT}/Include")
find_library(OPENNI2_LIBRARY OpenNI2 HINTS "${OPENNI2_ROOT}/Bin/x64-Release/" "${OPENNI2_ROOT}/lib")

find_package_handle_standard_args(OPENNI2 DEFAULT_MSG OPENNI2_LIBRARY OPENNI2_INCLUDE_DIR)

mark_as_advanced(OPENNI2_LIBRARY OPENNI2_INCLUDE_DIR)