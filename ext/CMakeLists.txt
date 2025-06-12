cmake_minimum_required(VERSION 3.22.1)

include(boost.cmake)

add_subdirectory(earcut.hpp)
add_subdirectory(glm)
add_subdirectory(stb)

set_property(TARGET glm PROPERTY FOLDER ext/glm)
