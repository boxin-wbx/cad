cmake_minimum_required(VERSION 3.5)
project(win32graphic)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c89")

set(SOURCE_FILES igp.c)
aux_source_directory(./libgraphics LIB_SOURCE)
include_directories("./include")
add_executable(win32graphic ${SOURCE_FILES} ${LIB_SOURCE})