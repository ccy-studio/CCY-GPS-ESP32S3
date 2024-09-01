file(GLOB_RECURSE SOURCES ${FRAMEWORK_ROOT_DIR}/swlv_gui/*.c)
file(GLOB_RECURSE CORE_SOURCES ${FRAMEWORK_ROOT_DIR}/swlv_core/*.c)
file(GLOB_RECURSE COMMON_SOURCES ${FRAMEWORK_ROOT_DIR}/swlv_common/*.c)

add_library(framework ${SOURCES} ${CORE_SOURCES} ${COMMON_SOURCES})
add_library(framework::all ALIAS framework)

target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(framework PUBLIC lvgl m pthread)

target_include_directories(framework PUBLIC
        ${FRAMEWORK_ROOT_DIR}/swlv_gui
        ${FRAMEWORK_ROOT_DIR}/swlv_core
        ${FRAMEWORK_ROOT_DIR}/swlv_common
)


