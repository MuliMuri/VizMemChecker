cmake_minimum_required(VERSION 3.10)

project(libs)

# Check if capstone exists
if(NOT IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/capstone)
    # capstone not found
    message(STATUS
    "\r\n
    [-] =====\t Can not find 'capstone' \t=====
    [+] =====\t Try to download 'capstone \t=====

    [!] If you want to specify a proxy, please run 'download.bat' separately, and add '-proxy ip:port'
    "
    )

    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        execute_process(COMMAND
            ${CMAKE_CURRENT_SOURCE_DIR}/download.bat -dir ${CMAKE_CURRENT_SOURCE_DIR} -proxy 127.0.0.1:1080
            COMMAND_ECHO STDOUT
        )
    endif()
    
endif()

aux_source_directory(
    ${CMAKE_CURRENT_SOURCE_DIR}/argtable3
    LIB_ARGTABLE3_DIR
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/capstone)

add_library(argtable3 STATIC ${LIB_ARGTABLE3_DIR})
