cmake_minimum_required(VERSION 2.8)
project(CPPLevenTest)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
endif(NOT MSVC)

add_executable(cpptest cppleven.cpp)
