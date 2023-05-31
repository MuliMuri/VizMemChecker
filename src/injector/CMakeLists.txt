cmake_minimum_required(VERSION 3.10)

project(injector)

aux_source_directory(
    ${CMAKE_CURRENT_SOURCE_DIR}
    INJECTOR_SRC_LIST
)

# set(
#     ASM_SOURCE_LIST
#     stubcode.asm
# )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${INJECTOR_SRC_LIST}
    # ${ASM_SOURCE_LIST}
)

target_link_libraries(
    ${PROJECT_NAME}
    common
)
