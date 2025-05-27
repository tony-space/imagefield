cmake_minimum_required(VERSION 3.22.1)

#add_subdirectory($ENV{BOOST_ROOT} boost-bin EXCLUDE_FROM_ALL)

set(BOOST_LIBS

#primary deps
	algorithm
	align
	any
	array
	assert
	atomic
	bind
	chrono
	concept_check
	config
	container
	container_hash
	conversion
	core
	crc
	date_time
	describe
	detail
	endian
	exception
	function
	function_types
	functional
	fusion
	integer
	intrusive
	io
	iterator
	lexical_cast
	math
	move
	mp11
	mpl
	multiprecision
	numeric/conversion
	optional
	phoenix
	pool
	predef
	preprocessor
	program_options 
	proto
	qvm
	ratio
	rational
	regex
	serialization
	smart_ptr
	spirit
	static_assert
	system
	thread
	throw_exception
	tokenizer
	tuple
	type_index
	type_traits
	typeof
	unordered
	utility
	variant
	variant2
	winapi

#secondary deps
	geometry
	range
	test
)

foreach(LIB IN LISTS BOOST_LIBS)
	add_subdirectory($ENV{BOOST_ROOT}/libs/${LIB} boost-bin/${LIB} EXCLUDE_FROM_ALL)
endforeach()

set_property(TARGET boost_assert PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_atomic PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_chrono PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_container PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_core PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_date_time PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_math PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_optional PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_program_options PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_serialization PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_system PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_thread PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_unordered PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_variant PROPERTY FOLDER ext/boost/)
set_property(TARGET boost_variant2 PROPERTY FOLDER ext/boost/)
