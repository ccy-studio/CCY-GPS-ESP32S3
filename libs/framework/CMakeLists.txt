cmake_minimum_required(VERSION 3.10)


message("is espidf platform  = ${ESP_PLATFORM}")

set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set(FRAMEWORK_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

if(ESP_PLATFORM)
    include(${CMAKE_CURRENT_LIST_DIR}/env_support/esp.cmake)
else()
    project(framework)
    include(../lvgl/CMakeLists.txt)
    include(${CMAKE_CURRENT_LIST_DIR}/env_support/custom.cmake)
endif()