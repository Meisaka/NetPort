cmake_minimum_required(VERSION 2.8)
project(NetPort)


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib )

set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(NOT CPPELEVEN_TESTED)
	message(STATUS "Testing for C++11...")
	try_compile(CPPELEVEN_SUPPORTED
		"${CMAKE_CURRENT_SOURCE_DIR}/envtest/bin"
		"${CMAKE_CURRENT_SOURCE_DIR}/envtest"
		CPPLevenTest
	)
	unset(CPPELEVEN_TESTED CACHE)
	set(CPPELEVEN_TESTED TRUE CACHE BOOL "C++11 test ran, set to false to re-run")
	mark_as_advanced(CPPELEVEN_TESTED)

	if(CPPELEVEN_SUPPORTED)
	message(STATUS "C++11 supported")
	option(CPPELEVEN "Use C++11" ON)
	else(CPPELEVEN_SUPPORTED)
	message(WARNING "C++11 is not supported")
	option(CPPELEVEN "Use C++11 anyway (test failed)" OFF)
	endif(CPPELEVEN_SUPPORTED)
endif(NOT CPPELEVEN_TESTED)

option(COPYSAFE "Allow NetPort classes to be copied" ON)
option(TESTAPPS "Build the MEI test client and server" OFF)

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
	"${INCLUDE_ROOT}/net/*.h*"
	"${SOURCE_ROOT}/config.in"
	"${SOURCE_ROOT}/*.h*"
	)
source_group("net" FILES ${NET_SRC})
source_group("includes" FILES ${NET_INC})

include_directories(${INCLUDE_ROOT})

add_library(netport STATIC
	${NET_SRC} ${NET_INC}
	)

if(TESTAPPS)
	file(GLOB NET_TEST_C
		"${SOURCE_ROOT}/test-client/*.c*"
		)
	file(GLOB NET_TEST_S
		"${SOURCE_ROOT}/test-server/*.c*"
		)
	add_executable(MEI-client ${NET_TEST_C})
	add_executable(MEI-server ${NET_TEST_S})

	target_link_libraries(MEI-client netport)
	target_link_libraries(MEI-server netport)
endif(TESTAPPS)

