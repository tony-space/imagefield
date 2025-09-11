cmake_minimum_required(VERSION 3.22.1)

project(IMFRuntimeCpu LANGUAGES CXX)

set(SRC
	include/imf/runtime/cpu/CpuRuntime.hpp
	include/imf/runtime/cpu/CpuTexture.hpp
	include/imf/runtime/cpu/pch.hpp

	detail/CpuOperationFactory.cpp
	detail/CpuOperationFactory.hpp
	detail/CpuRuntime.cpp
	detail/CpuSampler.cpp
	detail/CpuSampler.hpp
	detail/CpuTexture.cpp
	detail/NaiveGraphCompiler.cpp
	detail/NaiveGraphCompiler.hpp
	detail/Rasterizer.hpp

	detail/operations/CpuBicubicUpscale.cpp
	detail/operations/CpuColorSpaceConvert.cpp
	detail/operations/CpuMove.cpp
	detail/operations/CpuTransform.cpp
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})
add_library(ImageField::Runtime::CPU ALIAS ${PROJECT_NAME})

if (IMF_USE_PRECOMPILED_HEADERS)
	target_precompile_headers(${PROJECT_NAME} PRIVATE include/imf/runtime/cpu/pch.hpp)
endif()
set_property(TARGET ${PROJECT_NAME} PROPERTY UNITY_BUILD ${IMF_UNITY_BUILD})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED true)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER imagefield)

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PRIVATE IMF_CXX_FLAGS)
target_link_libraries(${PROJECT_NAME} PUBLIC ImageField::Core STB)
