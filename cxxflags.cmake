cmake_minimum_required(VERSION 3.22.1)

add_library(IMF_CXX_FLAGS INTERFACE)

target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Qpar>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/W4>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/WX>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/MP>)

target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:Clang>:-Werror>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:Clang>:-Wall>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:Clang>:-Wextra>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:Clang>:-Wno-tautological-pointer-compare>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:Clang>:-Wno-pointer-bool-conversion>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:Clang>:-Wmissing-prototypes>)

target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:AppleClang>:-Werror>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:AppleClang>:-Wall>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:AppleClang>:-Wextra>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:AppleClang>:-Wno-tautological-pointer-compare>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:AppleClang>:-Wno-pointer-bool-conversion>)
target_compile_options(IMF_CXX_FLAGS INTERFACE $<$<CXX_COMPILER_ID:AppleClang>:-Wmissing-prototypes>)
