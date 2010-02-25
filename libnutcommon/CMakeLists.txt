project(nutcommon)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 COMPONENTS QtCore QtNetwork QtDBus REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

set(SOURCES_MOC_H
	dbusmonitor.h
)
set(SOURCES_CPP
	macaddress.cpp
	config.cpp
	common.cpp
	device.cpp
	dbusmonitor.cpp
)

qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})

add_library(${PROJECT_NAME} STATIC ${SOURCES_CPP} ${SOURCES_MOC_CPP})

target_link_libraries(${PROJECT_NAME}
	${QT_LIBRARIES}
)
