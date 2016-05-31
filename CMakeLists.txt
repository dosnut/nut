cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
cmake_policy(SET CMP0022 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

PROJECT(network-utiltities
	VERSION 0.6.2
	LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${network-utiltities_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${network-utiltities_BINARY_DIR})

set(PACKAGE_NAME ${network-utiltities_PROJECT_NAME})
set(PACKAGE_VERSION ${network-utiltities_VERSION})
set(PACKAGE_SO_VERSION ${network-utiltities_VERSION}) # .so version for libnutclient and libnutcommon

include(Qt5Translate)
include(DefaultBuildType)
include(ExtraFlags)
include(ImportNetlink)
include(LanguageFlags)
include(LinkObjectLibrary)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(INTERPROCEDURAL_OPTIMIZATION ON)
set(INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

option(USE_WIRELESS "Enable support for wireless networks through wpa_supplicant." ON)
if(USE_WIRELESS)
	include(ImportLibIW)
else()
	# fake the lib.
	add_library("libiw" INTERFACE)
	add_definitions(-DNUT_NO_WIRELESS)
endif()

set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
set(CMAKE_INSTALL_INCLUDEDIR include CACHE PATH "Output directory for includes")

include_directories(${CMAKE_SOURCE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

# only qnut uses gui
set(QT_DONT_USE_QTGUI 1)

function(build_subdir)
	foreach(dir ${ARGN})
		add_subdirectory("${dir}" "${dir}-build")
	endforeach()
endfunction()

build_subdir(libnutcommon)

if(USE_WIRELESS)
	build_subdir(libnutwireless)
endif()

build_subdir(
	libnutclientbase
	libnutclient
	cnut
	nuts
	qnut
)
