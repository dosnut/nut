project(nutwireless)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 COMPONENTS QtCore QtNetwork QtDBus REQUIRED)
include(${QT_USE_FILE})

find_package(BISON)
find_package(FLEX)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES_MOC_H
# 	hwtypes.h
# 	wstypes.h
# 	types.h
	wpa_supplicant.h
	cwireless.h
	cwirelesshw.h
	cwexthw.h
	cnl80211.h
# 	cnetworkconfig.h
# 	conversion.h
# 	cconfigparser.h
)
set(SOURCES_CPP
	wstypes.cpp
	cws_net.cpp
	types.cpp
	wpa_supplicant.cpp
	parsers.cpp
	base.cpp
	cwireless.cpp
	cwexthw.cpp
	cwexthw_parse.cpp
	cnl80211.cpp
	cnetworkconfig.cpp
	conversion.cpp
	cconfigparser.cpp
)
set(SOURCES_C
	wpa_ctrl/wpa_ctrl.c
	wpa_ctrl/common.c
	wpa_ctrl/wpa_debug.c
	wpa_ctrl/os_unix.c
)

add_definitions(-DUSED_CMAKE -DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX)

bison_target(CONFIGPARSER_Y configparser.y ${CMAKE_CURRENT_BINARY_DIR}/configparser_yacc.cpp COMPILE_FLAGS "-p configparser")
flex_target(CONFIGPARSER_L configparser.l ${CMAKE_CURRENT_BINARY_DIR}/configparser_lex.cpp COMPILE_FLAGS "-Pconfigparser")
add_flex_bison_dependency(CONFIGPARSER_L CONFIGPARSER_Y)

qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})

add_library(${PROJECT_NAME} STATIC
	${SOURCES_C}
	${SOURCES_CPP}
	${SOURCES_MOC_CPP}
	${FLEX_CONFIGPARSER_L_OUTPUTS}
	${BISON_CONFIGPARSER_Y_OUTPUTS}
)

target_link_libraries(${PROJECT_NAME}
	${QT_LIBRARIES}
	${CMAKE_BINARY_DIR}/libnutcommon/libnutcommon.a
)
