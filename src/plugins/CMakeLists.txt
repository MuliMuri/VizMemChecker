cmake_minimum_required(VERSION 3.10)

project(plugins)

aux_source_directory(
	${CMAKE_CURRENT_SOURCE_DIR}
	PLUGINS_SRC_LIST
)

add_library(
	${PROJECT_NAME}
    STATIC
    ${PLUGINS_SRC_LIST}
)
