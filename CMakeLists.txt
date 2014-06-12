cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

MACRO(ADD_TARGET_PROPERTIES _target _name)
	SET(_properties)
	FOREACH(_prop ${ARGN})
		SET(_properties "${_properties} ${_prop}")
	ENDFOREACH(_prop)
	GET_TARGET_PROPERTY(_old_properties ${_target} ${_name})
	MESSAGE(STATUS "adding property to ${_target} ${_name}:" ${_properties})
	IF(NOT _old_properties)
		# in case it's NOTFOUND
		SET(_old_properties)
	ENDIF(NOT _old_properties)
	SET_TARGET_PROPERTIES(${_target} PROPERTIES ${_name} "${_old_properties} ${_properties}")
ENDMACRO(ADD_TARGET_PROPERTIES)


PROJECT(network-utiltities C CXX)
SET(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(PACKAGE_VERSION 0.5.8)

include(CMakeDetermineCCompiler)
include(CMakeDetermineCXXCompiler)

# default to RelWithDebInfo
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")

set(BASE_CXX_FLAGS "-std=c++11" CACHE STRING "Basic C++ flags to select required language features")
set(BASE_C_FLAGS "-std=c99" CACHE STRING "Basic C flags to select required language features")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(EXTRA_CXX_FLAGS "-Wall -Wno-long-long -Wextra -Wno-unused-parameter -pedantic" CACHE STRING "Extra flags used by the compiler during all build types.")
	set(EXTRA_EXE_LINKER_FLAGS "-Wl,--as-needed" CACHE STRING "Extra flags used by the linker.")
else("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(EXTRA_CXX_FLAGS "" CACHE STRING "Extra flags used by the compiler during all build types.")
	set(EXTRA_EXE_LINKER_FLAGS "" CACHE STRING "Extra flags used by the linker.")
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set(EXTRA_C_FLAGS "-Wall -Wno-long-long -Wextra -Wno-unused-parameter -pedantic" CACHE STRING "Extra flags used by the compiler during all build types.")
else("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set(EXTRA_C_FLAGS "" CACHE STRING "Extra flags used by the compiler during all build types.")
endif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BASE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EXTRA_EXE_LINKER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${BASE_C_FLAGS} ${EXTRA_C_FLAGS}")


option(USE_WIRELESS "Enable support for wireless networks through wpa_supplicant." ON)


add_subdirectory(libnutcommon libnutcommon)

add_subdirectory(cnut cnut)
add_dependencies(cnut nutcommon)

add_subdirectory(nuts nuts)
add_dependencies(nuts nutcommon)

if(USE_WIRELESS)
	add_subdirectory(libnutwireless libnutwireless)
	add_dependencies(nutwireless nutcommon)
else(USE_WIRELESS)
	add_definitions(-DQNUT_NO_WIRELESS -DLIBNUT_NO_WIRELESS)
endif(USE_WIRELESS)

add_subdirectory(libnutclient libnutclient)
add_dependencies(nutclient nutcommon)
if(USE_WIRELESS)
	add_dependencies(nutclient nutwireless)
endif(USE_WIRELESS)

add_subdirectory(qnut qnut)
add_dependencies(qnut nutclient)
