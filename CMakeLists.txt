cmake_minimum_required(VERSION 2.8)
project(NetPort)


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib )

set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include")

option(COPYSAFE "Allow NetPort classes to be copied" ON)
option(BUILD_SHARED_LIBS "Build shared (dynamic) library instead of static" OFF)

message(STATUS "Platform: '${CMAKE_SYSTEM_NAME}'")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(LINUX TRUE)
else(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(LINUX FALSE)
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

configure_file(${SOURCE_ROOT}/config.in ${SOURCE_ROOT}/config.h)

file(GLOB NET_SRC
	"${SOURCE_ROOT}/*.c*"
	)
file(GLOB NET_INC
	"${INCLUDE_ROOT}/include/net/*.h*"
	"${SOURCE_ROOT}/src/config.in"
	"${SOURCE_ROOT}/src/*.h*"
	)
source_group("net" FILES ${NET_SRC})
source_group("includes" FILES ${NET_INC})

include_directories(${INCLUDE_ROOT})

if(BUILD_SHARED_LIBS)
	add_definitions(-DNETPORT_SHARED -DNETPORT_EXPORT)
endif(BUILD_SHARED_LIBS)

add_library(netport
	${NET_SRC}
	)

