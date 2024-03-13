cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS    $ENV{LCD_LIB}
                            $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
                            ../external_libs/esp-idf-lib-v0.9.4/components)
include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Project name
project(smartspeaker_a4)


