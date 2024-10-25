cmake_minimum_required(VERSION 3.22.1)

add_subdirectory(test-suits)

project(master-test-suite LANGUAGES CXX)

set(SRC
	master-test-suite.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_executable(${PROJECT_NAME} ${SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED true)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests)

target_link_libraries(${PROJECT_NAME} PRIVATE IMF_CXX_FLAGS)
target_link_libraries(${PROJECT_NAME} PRIVATE test-suits)
