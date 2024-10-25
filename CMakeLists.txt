cmake_minimum_required(VERSION 3.22.1)

project(ImageField)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(IMF_USE_PRECOMPILED_HEADERS "Use precompiled headers" ON)
option(IMF_CPU_BACKEND "Enable CPU backend" ON)
option(IMF_OPENGL_BACKEND "Enable OpenGL backend" ON)
option(IMF_SANDBOXES "Enable sandboxes" ON)
option(IMF_TESTS "Enable Tests" ON)

MESSAGE(STATUS "Generating precompiled headers: " ${IMF_USE_PRECOMPILED_HEADERS})
MESSAGE(STATUS "Generating CPU backend: " ${IMF_OPENGL_BACKEND})
MESSAGE(STATUS "Generating OpenGL backend: " ${IMF_OPENGL_BACKEND})
MESSAGE(STATUS "Generating sandboxes: " ${IMF_SANDBOXES})
MESSAGE(STATUS "Generating tests: " ${IMF_TESTS})

if (CMAKE_SYSTEM_NAME STREQUAL Android OR CMAKE_SYSTEM_NAME STREQUAL iOS)
	set(IMF_DESKTOP_TARGET OFF)
else()
	set(IMF_DESKTOP_TARGET ON)
endif()

#third parties
add_subdirectory(ext)

include(cxxflags.cmake)

add_subdirectory(src)

#demos, tests, etc
if (IMF_SANDBOXES)
	add_subdirectory(sandboxes)
endif()

if (IMF_TESTS)
	add_subdirectory(tests)
endif()

include(InstallRequiredSystemLibraries)
