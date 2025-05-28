cmake_minimum_required(VERSION 3.22.1)

set(ACTIVE_RUNTIMES)

if (IMF_CPU_RUNTIME)
	add_subdirectory(cpu)
	set(ACTIVE_RUNTIMES ${ACTIVE_RUNTIMES} ImageField::Runtime::CPU)
endif()

set(ACTIVE_RUNTIMES ${ACTIVE_RUNTIMES} PARENT_SCOPE)
