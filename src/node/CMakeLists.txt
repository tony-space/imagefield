cmake_minimum_required(VERSION 3.22.1)

project(IMFnode LANGUAGES CXX)

set(SRC
	detail/register_standard_graph_nodes.cpp

	include/imf/core/node/pch.hpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})
add_library(ImageField::Node ALIAS ${PROJECT_NAME})

target_precompile_headers(${PROJECT_NAME} PRIVATE include/imf/core/node/pch.hpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED true)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER imagefield)

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PRIVATE IMF_CXX_FLAGS)
target_link_libraries(${PROJECT_NAME} PUBLIC ImageField::Core)
