cmake_minimum_required(VERSION 3.22.1)

add_subdirectory(glm)
set_property(TARGET glm PROPERTY FOLDER ext/glm)

if (DESKTOP_TARGET AND IMF_OPENGL_BACKEND)
	add_subdirectory(glfw)
endif()
