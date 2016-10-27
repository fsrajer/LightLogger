cmake_minimum_required(VERSION 2.6)

project(LightLogger)

include(FindPackageHandleStandardArgs)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

macro(CANONIFY_BOOL var)
  if (${var})
    set(${var} TRUE)
  else()
    set(${var} FALSE)
  endif()
endmacro()

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(RealSense REQUIRED)

find_package(ZLIB QUIET)
CANONIFY_BOOL(ZLIB_FOUND)
message(STATUS "zlib found: ${ZLIB_FOUND}")
option(WITH_ZLIB "Build with zlib support? (strongly recommended)" ${ZLIB_FOUND})
if(WITH_ZLIB)
  add_definitions(-DWITH_ZLIB)
  set(EXTRA_INCL ${EXTRA_INCL} ${ZLIB_INCLUDE_DIR})
  set(EXTRA_LIBS ${EXTRA_LIBS} ${ZLIB_LIBRARY})
endif()

find_package(JPEG QUIET)
CANONIFY_BOOL(JPEG_FOUND)
message(STATUS "libjpeg found: ${JPEG_FOUND}")
option(WITH_JPEG "Build with libjpeg support? (strongly recommended)" ${JPEG_FOUND})
if(WITH_JPEG)
  add_definitions(-DWITH_JPEG)
  set(EXTRA_INCL ${EXTRA_INCL} ${JPEG_INCLUDE_DIR})
  set(EXTRA_LIBS ${EXTRA_LIBS} ${JPEG_LIBRARY})
endif()

include_directories(
    ${GLUT_INCLUDE_DIR}
	${REALSENSE_INCLUDE_DIR}
	${EXTRA_INCL}
)
  
add_executable(LightLogger 
				main.cpp
				CameraInterface.h
				RealSenseInterface.cpp
				RealSenseInterface.h
				Logger.cpp
				Logger.h
)

target_link_libraries(LightLogger
					  ${GLUT_LIBRARY}
					  ${REALSENSE_LIBRARY}
					  ${EXTRA_LIBS}
)

install(TARGETS LightLogger 
		DESTINATION bin)
