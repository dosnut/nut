find_package(BISON)
find_package(FLEX)

add_definitions(-DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX)

bison_target(CONFIGPARSER_Y configparser.y ${CMAKE_CURRENT_BINARY_DIR}/configparser_yacc.cpp COMPILE_FLAGS "-p configparser")
flex_target(CONFIGPARSER_L configparser.l ${CMAKE_CURRENT_BINARY_DIR}/configparser_lex.cpp COMPILE_FLAGS "-Pconfigparser")
add_flex_bison_dependency(CONFIGPARSER_L CONFIGPARSER_Y)

add_library(nutwireless OBJECT
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
	cwirelesshw.h

	wpa_ctrl/wpa_ctrl.c
	wpa_ctrl/common.c
	wpa_ctrl/wpa_debug.c
	wpa_ctrl/os_unix.c

	${FLEX_CONFIGPARSER_L_OUTPUTS}
	${BISON_CONFIGPARSER_Y_OUTPUTS}
)

object_link_libraries(nutwireless PUBLIC
	nutcommon
	libnl
	libnl-genl
	Qt5::Network
	Qt5::Core
)

file(GLOB NUTWIRELESS_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${NUTWIRELESS_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutwireless)
