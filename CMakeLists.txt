cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS    #$ENV{LCD_LIB}
                            $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
                            #C:/Users/Cam/ESP-projects/smartspeaker_a4/esp-idf-lib/components
                            ../external_libs/esp-idf-lib-v0.9.4/components)
include($ENV{ADF_PATH}/CMakeLists.txt) # Add ADF_PATH to env. Make sure that ADF is v2.6
include($ENV{IDF_PATH}/tools/cmake/project.cmake) # Add IDF_PATH to env. Make sure that IDF is v4.4

project(smartspeaker_a4)
