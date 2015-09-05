project(nuts)

include(${QT_USE_FILE})

find_package(BISON)
find_package(FLEX)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES_MOC_H
	device.h
# 	configparser.h
# 	configparser_p.h
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
	configparser.cpp
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

flex_target(CONFIGPARSER_L configparser.l ${CMAKE_CURRENT_BINARY_DIR}/configparser_lex.cpp COMPILE_FLAGS "-Pconfigparser")

qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})


add_executable(${PROJECT_NAME}
	${SOURCES_C}
	${SOURCES_CPP}
	${SOURCES_MOC_CPP}
	${FLEX_CONFIGPARSER_L_OUTPUTS}
	${BISON_CONFIGPARSER_Y_OUTPUTS}
)

target_link_libraries(${PROJECT_NAME}
	nutcommon
	libnl
	libnl-route
	${QT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)
