file(GLOB_RECURSE SOURCES ${FRAMEWORK_ROOT_DIR}/swlv_gui/*.c)
file(GLOB_RECURSE CORE_SOURCES ${FRAMEWORK_ROOT_DIR}/swlv_core/*.c)
file(GLOB_RECURSE COMMON_SOURCES ${FRAMEWORK_ROOT_DIR}/swlv_common/*.c)

idf_component_register(SRCS ${SOURCES} ${CORE_SOURCES} ${COMMON_SOURCES}
    INCLUDE_DIRS ${FRAMEWORK_ROOT_DIR}/swlv_gui/ ${FRAMEWORK_ROOT_DIR}/swlv_core/ ${FRAMEWORK_ROOT_DIR}/swlv_common/
    REQUIRES "lvgl"
)