cmake_minimum_required(VERSION 3.22.1)

add_subdirectory(glm)
add_subdirectory(core)
add_subdirectory(node)
add_subdirectory(runtime)

set(ImageField ImageField::Core ImageField::Node ${ACTIVE_RUNTIMES} PARENT_SCOPE)
