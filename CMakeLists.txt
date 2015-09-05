cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(SET CMP0022 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

PROJECT(network-utiltities C CXX)
SET(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(PACKAGE_VERSION 0.5.8)
set(PACKAGE_SO_VERSION 0) # .so version for libnutclient and libnutcommon

include(DefaultBuildType)
include(ExtraFlags)
include(ImportNetlink)
include(LanguageFlags)
include(LinkObjectLibrary)
include(MyQtTranslation)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

add_subdirectory(libnutcommon)

if(USE_WIRELESS)
	add_subdirectory(libnutwireless)
endif()
add_subdirectory(libnutclient)

add_subdirectory(cnut)
add_subdirectory(nuts)
add_subdirectory(qnut)

# sadly touches files always even when nothing changed
option(UPDATE_TRANSLATIONS "Automatically update translations in source directory" OFF)

if(UPDATE_TRANSLATIONS)
  add_dependencies(nutclient update-translations)
  add_dependencies(qnut update-translations)
endif()
