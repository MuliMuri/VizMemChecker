cmake_minimum_required(VERSION 3.10)

project(test)

file(GLOB_RECURSE TEST_SOURCE_DIR "*.c")

foreach(TSRC ${TEST_SOURCE_DIR})
    get_filename_component(TSRC_NAME ${TSRC} NAME_WE)
    add_executable(${TSRC_NAME} "${TSRC}")
    target_link_libraries(
        ${TSRC_NAME} PRIVATE
        common core
    )
    add_test(NAME "${TSRC_NAME}" COMMAND ${TSRC_NAME})
endforeach()
