#####################################
# Project-wide settings
#####################################
cmake_minimum_required(VERSION 3.15)

project(syscall_intercept)

#####################################
# CMake module imports
#####################################
include(ExternalProject)

#####################################
# Import external project from git
#####################################
ExternalProject_Add(syscall_intercept
        GIT_REPOSITORY git@github.com:alpha-unito/syscall_intercept.git
	GIT_TAG sifive
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        CMAKE_ARGS
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
