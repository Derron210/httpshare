
# Minimum CMake version required 
cmake_minimum_required(VERSION 3.10)
# Project name 
project(httpshare C)
# Add the source file 
set(SRC httpshare.c) 
# Set output executable name 
set(EXECUTABLE_NAME httpshare)

# Add the executable 
add_executable(${EXECUTABLE_NAME} ${SRC})