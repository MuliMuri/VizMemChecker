cmake_minimum_required(VERSION 3.10)

project("common")

aux_source_directory(
    ${CMAKE_CURRENT_SOURCE_DIR}
    COMMON_SRC_LIST
)

add_library(
    ${PROJECT_NAME}
    STATIC
    ${COMMON_SRC_LIST}
)
