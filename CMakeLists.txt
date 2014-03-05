cmake_minimum_required(VERSION 2.8)
project(NetPort)


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib )

set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include")

option(COPYSAFE "Allow NetPort classes to be copied" ON)

configure_file(${SOURCE_ROOT}/config.in ${SOURCE_ROOT}/config.h)

file(GLOB NET_SRC
	"${SOURCE_ROOT}/*.c*"
	)
source_group("net" FILES ${NET_SRC})

include_directories(${INCLUDE_ROOT})

add_library(netport STATIC
	${NET_SRC}
	)

