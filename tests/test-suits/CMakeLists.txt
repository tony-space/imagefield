cmake_minimum_required(VERSION 3.22.1)

project(test-suits LANGUAGES CXX)

set(SRC
	detail/basic-test-suite.cpp

	include/imf/tests/pch.hpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})

if (IMF_USE_PRECOMPILED_HEADERS)
	target_precompile_headers(${PROJECT_NAME} PRIVATE include/imf/tests/pch.hpp)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY UNITY_BUILD ${IMF_UNITY_BUILD})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED true)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests)

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PRIVATE IMF_CXX_FLAGS)
target_link_libraries(${PROJECT_NAME} PUBLIC ${ImageField} Boost::included_unit_test_framework)

target_link_options(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:${PROJECT_NAME}>)
target_link_options(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:AppleClang>:-Wl,-all_load>)
target_link_options(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:Clang>:-Wl,-whole-archive  $<TARGET_FILE:${PROJECT_NAME}> -Wl,-no-whole-archive>)
