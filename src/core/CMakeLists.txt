cmake_minimum_required(VERSION 3.10)

project(core)

aux_source_directory(
    ${CMAKE_CURRENT_SOURCE_DIR}
    CORE_SRC_LIST
)

add_library(
    ${PROJECT_NAME}
    STATIC
    ${CORE_SRC_LIST}
)

target_link_libraries(
    ${PROJECT_NAME}
    common
    capstone
)
