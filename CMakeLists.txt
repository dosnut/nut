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


PROJECT(network-utiltities)
SET(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(PACKAGE_VERSION 0.5.7)

option(USE_WIRELESS "Enable support for wireless networks through wpa_supplicant." ON)

set(DEBUG_FLAGS "-Wall -Wno-long-long -pedantic")
set(CMAKE_C_FLAGS "-std=c99")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${DEBUG_FLAGS}")

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
