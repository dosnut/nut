project(nuts)

find_package(BISON)
find_package(FLEX)

flex_target(CONFIGPARSER_L configparser.l ${CMAKE_CURRENT_BINARY_DIR}/configparser_lex.cpp COMPILE_FLAGS "-Pconfigparser")

add_executable(${PROJECT_NAME}
	arp.cpp
	arp.h
	configparser.cpp
	configparser.h
	configparser_p.h
	dbus.cpp
	dbus.h
	device.cpp
	device.h
	dhcppacket.cpp
	dhcppacket.h
	events.cpp
	events.h
	exception.h
	hardware.cpp
	hardware.h
	log.cpp
	log.h
	main.cpp
	processmanager.cpp
	processmanager.h
	random.cpp
	random.h
	sighandler.cpp
	sighandler.h
	timecls.cpp
	timecls.h

	configparser.l

	${FLEX_CONFIGPARSER_L_OUTPUTS}
	${BISON_CONFIGPARSER_Y_OUTPUTS}
)

target_link_libraries(${PROJECT_NAME}
	nutnetlink
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)
