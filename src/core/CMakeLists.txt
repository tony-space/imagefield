cmake_minimum_required(VERSION 3.22.1)

find_package(Threads REQUIRED)

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
find_package(Boost REQUIRED)

project(IMFCore LANGUAGES CXX)

set(SRC
  detail/dummy.cpp
  
  include/imagefield/core/pch.hpp 	
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})
add_library(ImageField::Core ALIAS ${PROJECT_NAME})

target_precompile_headers(${PROJECT_NAME} PRIVATE include/imagefield/core/pch.hpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED true)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER imagefield/core)

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PRIVATE IMF_CXX_FLAGS)
target_link_libraries(${PROJECT_NAME} PUBLIC
	Threads::Threads
	Boost::boost
)
if(CMAKE_SYSTEM_NAME STREQUAL Android)
	target_link_libraries(${PROJECT_NAME} PUBLIC log android)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
	target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
