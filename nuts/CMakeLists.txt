project(nuts)

find_package(BISON)
find_package(FLEX)

flex_target(CONFIGPARSER_L configparser.l ${CMAKE_CURRENT_BINARY_DIR}/configparser_lex.cpp COMPILE_FLAGS "-Pconfigparser")

add_executable(${PROJECT_NAME}
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

	hardware_ext.c

	${FLEX_CONFIGPARSER_L_OUTPUTS}
	${BISON_CONFIGPARSER_Y_OUTPUTS}
)

target_link_libraries(${PROJECT_NAME}
	nutcommon
	libnl
	libnl-route
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)
