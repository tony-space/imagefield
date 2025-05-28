cmake_minimum_required(VERSION 3.22.1)

#find_package(Threads REQUIRED)

project(IMFCore LANGUAGES CXX)

set(SRC
	detail/node/PlaceholderNode.cpp
	detail/node/SinkNode.cpp

	detail/DataFlow.cpp
	detail/GraphNode.cpp
	detail/GraphNodeFactory.cpp
	detail/log-device-std.cpp
	detail/log.cpp
	detail/Region.cpp
	detail/RuntimeFactory.cpp
	detail/unique_id_t.cpp

	include/imf/core/node/PlaceholderNode.hpp
	include/imf/core/node/SinkNode.hpp

	include/imf/core/BoundingBox.hpp
	include/imf/core/core.hpp
	include/imf/core/DataFlow.hpp
	include/imf/core/GraphNode.hpp
	include/imf/core/GraphNodeFactory.hpp
	include/imf/core/IRuntime.hpp
	include/imf/core/iterator_range.hpp
	include/imf/core/log.hpp
	include/imf/core/pch.hpp
	include/imf/core/Region.hpp
	include/imf/core/RuntimeFactory.hpp
	include/imf/core/TypeID.hpp
	include/imf/core/TypeQualifier.hpp
	include/imf/core/unique_id_t.hpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})
add_library(ImageField::Core ALIAS ${PROJECT_NAME})

if (IMF_USE_PRECOMPILED_HEADERS)
	target_precompile_headers(${PROJECT_NAME} PRIVATE include/imf/core/pch.hpp)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED true)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER imagefield)

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PRIVATE IMF_CXX_FLAGS)

target_link_libraries(${PROJECT_NAME} PUBLIC
#	Threads::Threads
	Boost::range
	Boost::geometry
	ImageField::glm
	ImageField::earcut
)

if (IMF_CPU_RUNTIME)
	target_compile_definitions(${PROJECT_NAME} PUBLIC IMF_CPU_RUNTIME_AVAILABLE)
endif()

# if(CMAKE_SYSTEM_NAME STREQUAL Android)
# 	target_link_libraries(${PROJECT_NAME} PUBLIC log android)
# endif()

# if(CMAKE_SYSTEM_NAME STREQUAL Windows)
# 	target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
# endif()
