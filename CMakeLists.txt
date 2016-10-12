cmake_minimum_required(VERSION 2.6)

project(LoggerRealSense)

include(FindPackageHandleStandardArgs)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(RealSense REQUIRED)

include_directories(
    ${GLUT_INCLUDE_DIR}
	${REALSENSE_INCLUDE_DIR}
)
  
add_executable(LoggerRealSense main.cpp)

target_link_libraries(LoggerRealSense
					  ${GLUT_LIBRARY}
					  ${REALSENSE_LIBRARY}
)

install(TARGETS LoggerRealSense 
		DESTINATION bin)
