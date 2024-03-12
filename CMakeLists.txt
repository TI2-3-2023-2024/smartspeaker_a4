# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# include ESP CMake tools
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set external libraries dir (add a directory that has your libraries 
# and comment the ones not present)
# TODO have a common relative path with documentation of setup
# set(EXTRA_COMPONENT_DIRS $ENV{LCD_LIB})
# set(EXTRA_COMPONENT_DIRS C:/Projects/esp/external_libs/esp-idf-lib-v0.9.4/components)
set(EXTRA_COMPONENT_DIRS ../external_libs/esp-idf-lib-v0.9.4/components)

# Project name
project(smartspeaker_a4)


