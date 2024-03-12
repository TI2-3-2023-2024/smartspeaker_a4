# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS $ENV{LCD_LIB})
set(EXTRA_COMPONENT_DIRS ../external_libs/esp-idf-lib-v0.9.4/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(app-template)
