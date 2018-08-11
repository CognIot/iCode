# CMakeLists text file of the project. 
# It is the input for cmake command, cmake is cross platform makefiles generator

cmake_minimum_required (VERSION 2.8.11)

#set (CMAKE_C_FLAGS "std=c99")

project (rfal)

add_subdirectory (rfal)
add_subdirectory (applications)


