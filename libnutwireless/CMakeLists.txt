find_package(BISON)
find_package(FLEX)

add_definitions(-DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX)

bison_target(CONFIGPARSER_Y configparser.y ${CMAKE_CURRENT_BINARY_DIR}/configparser_yacc.cpp COMPILE_FLAGS "-p configparser")
flex_target(CONFIGPARSER_L configparser.l ${CMAKE_CURRENT_BINARY_DIR}/configparser_lex.cpp COMPILE_FLAGS "-Pconfigparser")
add_flex_bison_dependency(CONFIGPARSER_L CONFIGPARSER_Y)

add_library(nutwireless OBJECT
	base.cpp
	cconfigparser.cpp
	cconfigparser.h
	cnetworkconfig.cpp
	cnetworkconfig.h
	cnl80211.cpp
	cnl80211.h
	conversion.cpp
	conversion.h
	cwexthw.cpp
	cwexthw.h
	cwexthw_parse.cpp
	cwireless.cpp
	cwireless.h
	cwirelesshw.h
	cws_net.cpp
	parsers.cpp
	types.cpp
	types.h
	wpa_supplicant.cpp
	wpa_supplicant.h
	wstypes.cpp
	wstypes.h

	wpa_ctrl/build_config.h
	wpa_ctrl/common.c
	wpa_ctrl/common.h
	wpa_ctrl/os.h
	wpa_ctrl/os_unix.c
	wpa_ctrl/wpa_ctrl.c
	wpa_ctrl/wpa_ctrl.h
	wpa_ctrl/wpa_debug.c
	wpa_ctrl/wpa_debug.h

	configparser.l
	configparser.y

	${FLEX_CONFIGPARSER_L_OUTPUTS}
	${BISON_CONFIGPARSER_Y_OUTPUTS}
)

object_link_libraries(nutwireless PUBLIC
	nutnetlink
	Qt5::Network
	Qt5::Core
)

file(GLOB NUTWIRELESS_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${NUTWIRELESS_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutwireless)
