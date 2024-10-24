cmake_minimum_required(VERSION 3.22.1)

project(cli-sandbox LANGUAGES CXX)

set(SRC
	main.cpp
	pch.hpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_executable(${PROJECT_NAME} ${SRC})

if (IMF_USE_PRECOMPILED_HEADERS)
	target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED true)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER demo)

target_link_libraries(${PROJECT_NAME} PRIVATE IMF_CXX_FLAGS)
target_link_libraries(${PROJECT_NAME} PRIVATE ${ImageField})
