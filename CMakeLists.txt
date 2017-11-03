cmake_minimum_required(VERSION 3.8)
project(ListDir)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        file_stats.c
        file_stats.h
        listdir.c
        listdir.h)

add_executable(ListDir ${SOURCE_FILES})