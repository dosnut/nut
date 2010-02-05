project(nuts)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 COMPONENTS QtCore QtNetwork QtDBus REQUIRED)
include(${QT_USE_FILE})

find_package(BISON)
find_package(FLEX)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES_MOC_H
	device.h
# 	config.h
	hardware.h
	
	sighandler.h
# 	log.h
# 	dhcppacket.h
# 	exception.h
	
	dbus.h
	arp.h
# 	random.h
	events.h
)
set(SOURCES_CPP
	main.cpp
	device.cpp
	config.cpp
	hardware.cpp
	
	sighandler.cpp
	log.cpp
	dhcppacket.cpp
	
	dbus.cpp
	arp.cpp
	random.cpp
	events.cpp
)
set(SOURCES_C
	hardware_ext.c
)

add_definitions(-DUSED_CMAKE)

bison_target(CONFIGPARSER_Y configparser.y ${CMAKE_CURRENT_BINARY_DIR}/configparser_yacc.cpp COMPILE_FLAGS "-p configparser")
flex_target(CONFIGPARSER_L configparser.l ${CMAKE_CURRENT_BINARY_DIR}/configparser_lex.cpp COMPILE_FLAGS "-Pconfigparser")
add_flex_bison_dependency(CONFIGPARSER_L CONFIGPARSER_Y)

qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})


add_executable(${PROJECT_NAME}
	${SOURCES_C}
	${SOURCES_CPP}
	${SOURCES_MOC_CPP}
	${FLEX_CONFIGPARSER_L_OUTPUTS}
	${BISON_CONFIGPARSER_Y_OUTPUTS}
)

add_target_properties(${PROJECT_NAME} LINK_FLAGS -lnl)

target_link_libraries(${PROJECT_NAME}
	${QT_QTCORE_LIBRARY}
	${QT_QTNETWORK_LIBRARY}
	${QT_QTDBUS_LIBRARY}
	${CMAKE_BINARY_DIR}/libnutcommon/libnutcommon.a
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)
