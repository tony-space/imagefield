cmake_minimum_required(VERSION 3.22.1)

#add_subdirectory($ENV{BOOST_ROOT} boost-bin EXCLUDE_FROM_ALL)

set(BOOST_LIBS

#primary deps
	algorithm
	array
	assert
	bind
	concept_check
	config
	container_hash
	conversion
	core
	describe
	detail
	exception
	function
	function_types
	functional
	fusion
	io
	iterator
	move
	mpl
	mp11
	numeric/conversion
	optional
	predef
	preprocessor
	regex
	static_assert
	smart_ptr
	throw_exception
	tuple
	type_traits
	typeof
	unordered
	utility

#secondary deps
	range
	test
)

foreach(LIB IN LISTS BOOST_LIBS)
	add_subdirectory($ENV{BOOST_ROOT}/libs/${LIB} boost-bin/${LIB} EXCLUDE_FROM_ALL)
endforeach()

set_property(TARGET boost_assert PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_core PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_unordered PROPERTY FOLDER ext/boost/)
