cmake_minimum_required(VERSION 3.10)

project(VizMemChecker)

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libs/capstone/include)

aux_source_directory(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    MAIN_SRC_LIST
)

add_subdirectory(src/common)
add_subdirectory(src/libs)
add_subdirectory(src/core)
add_subdirectory(src/injector)

add_executable(
    ${PROJECT_NAME}
    ${MAIN_SRC_LIST}
)

add_dependencies(
    ${PROJECT_NAME}
    common
    core
)

target_link_libraries(
    ${PROJECT_NAME}
    argtable3
    common
    core
)
