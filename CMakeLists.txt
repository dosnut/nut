cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(SET CMP0022 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

PROJECT(network-utiltities
	VERSION 0.5.9
	LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${network-utiltities_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${network-utiltities_BINARY_DIR})

set(PACKAGE_NAME ${network-utiltities_PROJECT_NAME})
set(PACKAGE_VERSION ${network-utiltities_VERSION})
set(PACKAGE_SO_VERSION ${network-utiltities_VERSION}) # .so version for libnutclient and libnutcommon

include(DefaultBuildType)
include(ExtraFlags)
include(ImportNetlink)
include(LanguageFlags)
include(LinkObjectLibrary)
include(MyQtTranslation)

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

find_package(Qt4 COMPONENTS QtCore QtGui QtDbus QtNetwork REQUIRED)
# only qnut uses gui
set(QT_DONT_USE_QTGUI 1)

function(build_subdir)
	foreach(dir ${ARGN})
		add_subdirectory("${dir}" "${dir}-build")
	endforeach()
endfunction()

if(USE_WIRELESS)
	build_subdir(libnutwireless)
endif()

build_subdir(
	libnutcommon
	libnutclient
	cnut
	nuts
	qnut
)

# sadly touches files always even when nothing changed
option(UPDATE_TRANSLATIONS "Automatically update translations in source directory" OFF)

if(UPDATE_TRANSLATIONS)
	add_dependencies(nutclient update-translations)
	add_dependencies(qnut update-translations)
endif()
