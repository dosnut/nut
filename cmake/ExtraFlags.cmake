include(CMakeDetermineCCompiler)
include(CMakeDetermineCXXCompiler)

set(_default_c_flags)
set(_default_cxx_flags)
set(_default_linker_flags)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set(_default_c_flags "-Wall -Wno-long-long -Wextra -Wno-unused-parameter -pedantic")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(_default_cxx_flags "-Wall -Wno-long-long -Wextra -Wno-unused-parameter -pedantic")
	set(_default_linker_flags "-Wl,--as-needed")
endif()

set(EXTRA_C_FLAGS "${_default_c_flags}" CACHE STRING "Extra C flags used by the compiler for all build types.")
set(EXTRA_CXX_FLAGS "${_default_cxx_flags}" CACHE STRING "Extra C++ flags used by the compiler for all build types.")
set(EXTRA_EXE_LINKER_FLAGS "${_default_linker_flags}" CACHE STRING "Extra flags used by the linker.")
mark_as_advanced(FORCE EXTRA_C_FLAGS EXTRA_CXX_FLAGS EXTRA_EXE_LINKER_FLAGS)

unset(_default_c_flags)
unset(_default_cxx_flags)
unset(_default_linker_flags)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EXTRA_EXE_LINKER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_FLAGS}")
