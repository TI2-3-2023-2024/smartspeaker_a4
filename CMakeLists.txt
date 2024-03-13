cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS    $ENV{LCD_LIB}
                            $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)
include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set external libraries dir (add a directory that has your libraries 
# and comment the ones not present)
# TODO have a common relative path with documentation of setup
# set(EXTRA_COMPONENT_DIRS $ENV{LCD_LIB})
# set(EXTRA_COMPONENT_DIRS C:/Projects/esp/external_libs/esp-idf-lib-v0.9.4/components)
set(EXTRA_COMPONENT_DIRS ../external_libs/esp-idf-lib-v0.9.4/components)

# Project name
project(smartspeaker_a4)


