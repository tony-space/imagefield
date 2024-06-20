cmake_minimum_required(VERSION 3.22.1)

project(ImageField)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(IMF_OPENGL_BACKEND "Enable OpenGL backend" ON)
option(IMF_SANDBOXES "Enable sandboxes" ON)

MESSAGE(STATUS "Generating OpenGL backend: " ${IMF_OPENGL_BACKEND})
MESSAGE(STATUS "Generating sandboxes: " ${IMF_SANDBOXES})

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


include(InstallRequiredSystemLibraries)
