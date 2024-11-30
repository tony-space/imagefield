cmake_minimum_required(VERSION 3.22.1)

project(earcut LANGUAGES CXX)

set(SRC
	include/mapbox/earcut.hpp
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} INTERFACE ${SRC})
add_library(ImageField::earcut ALIAS ${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ext)

target_include_directories(${PROJECT_NAME} INTERFACE "include")
