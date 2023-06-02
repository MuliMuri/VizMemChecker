cmake_minimum_required(VERSION 3.10)

project(utils)

aux_source_directory(
	${CMAKE_CURRENT_SOURCE_DIR}
	UTILS_SOURCE_DIR
)

add_library(
	${PROJECT_NAME}
	STATIC
	${UTILS_SOURCE_DIR}
)
