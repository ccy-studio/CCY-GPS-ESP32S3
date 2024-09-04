# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
# https://docs.espressif.com/projects/esp-idf/zh_CN/v5.1.4/esp32s3/api-guides/build-system.html
# https://docs.espressif.com/projects/esp-idf/zh_CN/v5.1.4/esp32s3/api-guides/build-system.html#component-requirements
cmake_minimum_required(VERSION 3.16)

#声明需要扫描搜索的外部组件
set(EXTRA_COMPONENT_DIRS "../libs/")

add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(saisiawa_gps_esp32s3)
