cmake_minimum_required(VERSION 2.6)

project(LoggerRealSense)

add_executable(LoggerRealSense main.cpp)

install(TARGETS LoggerRealSense 
		DESTINATION bin)
