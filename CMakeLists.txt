cmake_minimum_required(VERSION 2.8.10 FATAL_ERROR)
cmake_policy(SET CMP0022 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

PROJECT(network-utiltities C CXX)
SET(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(PACKAGE_VERSION 0.5.8)
set(PACKAGE_SO_VERSION 0) # .so version for libnutclient and libnutcommon

include(CMakeDetermineCCompiler)
include(CMakeDetermineCXXCompiler)
include(MyQtTranslation)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
set(CMAKE_INSTALL_INCLUDEDIR include CACHE PATH "Output directory for includes")

find_package(Qt4 COMPONENTS QtCore QtGui QtDbus QtNetwork REQUIRED)
# only qnut uses gui
set(QT_DONT_USE_QTGUI 1)

find_library(LIB_NL nl)

add_subdirectory(libnutcommon)
add_subdirectory(libnutclient)

if(USE_WIRELESS)
	add_subdirectory(libnutwireless)
else(USE_WIRELESS)
	add_definitions(-DNUT_NO_WIRELESS)
endif(USE_WIRELESS)

add_subdirectory(cnut)
add_subdirectory(nuts)
add_subdirectory(qnut)

# sadly touches files always even when nothing changed
option(UPDATE_TRANSLATIONS "Automatically update translations in source directory" OFF)

if(UPDATE_TRANSLATIONS)
  add_dependencies(nutclient update-translations)
  add_dependencies(qnut update-translations)
endif(UPDATE_TRANSLATIONS)
