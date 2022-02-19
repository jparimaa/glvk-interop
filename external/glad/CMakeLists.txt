cmake_minimum_required(VERSION 3.18)

project(Glad VERSION 0.0.1 LANGUAGES C)

file(GLOB SOURCES src/*.c include/*.h)

add_library(glad ${SOURCES})
target_include_directories(glad PUBLIC include/)