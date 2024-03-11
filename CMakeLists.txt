cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS    $ENV{LCD_LIB}
                            $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
                            C:/Users/Cam/ESP-projects/smartspeaker_a4/esp-idf-lib/components)
include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(app-template)
