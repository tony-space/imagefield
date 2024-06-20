cmake_minimum_required(VERSION 3.22.1)

add_subdirectory(core)

set(
  ImageField
  
	ImageField::Core
	${ACTIVE_BACKENDS}
	
	PARENT_SCOPE)
