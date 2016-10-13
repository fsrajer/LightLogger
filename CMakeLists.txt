cmake_minimum_required(VERSION 2.6)

project(LoggerRealSense)

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

include_directories(
    ${GLUT_INCLUDE_DIR}
	${REALSENSE_INCLUDE_DIR}
	${EXTRA_INCL}
)
  
add_executable(LoggerRealSense 
				main.cpp
				CameraInterface.h
				RealSenseInterface.cpp
				RealSenseInterface.h
				Logger.cpp
				Logger.h
)

target_link_libraries(LoggerRealSense
					  ${GLUT_LIBRARY}
					  ${REALSENSE_LIBRARY}
					  ${EXTRA_LIBS}
)

install(TARGETS LoggerRealSense 
		DESTINATION bin)
