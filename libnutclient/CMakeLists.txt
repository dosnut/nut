project(nutclient)

find_package(Qt4 COMPONENTS QtCore QtNetwork QtDBus REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

set(SOURCES_MOC_H
	server_proxy.h
	cinterface.h
	cenvironment.h
	cdevice.h
	cdevicemanager.h
	clibnut.h
	clog.h
)
set(SOURCES_CPP
	server_proxy.cpp
	cinterface.cpp
	cenvironment.cpp
	cdevice.cpp
	cdevicemanager.cpp
	clibnut.cpp
	clog.cpp
)

SET(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
SET(CMAKE_INSTALL_INCLUDEDIR include CACHE PATH "Output directory for includes")

qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})

add_definitions(-DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX)

add_library(${PROJECT_NAME} SHARED ${SOURCES_CPP} ${SOURCES_MOC_CPP})

add_target_properties(nutcommon COMPILE_FLAGS -fPIC)
add_target_properties(nutwireless COMPILE_FLAGS -fPIC)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION 0)

find_library(LIB_IW iw)
find_library(LIB_NL nl)

target_link_libraries(${PROJECT_NAME}
	${nutwireless_BINARY_DIR}/libnutwireless.a
	${nutcommon_BINARY_DIR}/libnutcommon.a
	${LIB_IW}
	${LIB_NL}
	${QT_QTCORE_LIBRARY}
	${QT_QTNETWORK_LIBRARY}
	${QT_QTDBUS_LIBRARY}
)

file(GLOB NUTCOMMON_H ${nutcommon_SOURCE_DIR}/*.h)
file(GLOB NUTWIRELESS_H ${nutwireless_SOURCE_DIR}/*.h)
file(GLOB NUTCLIENT_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${NUTCOMMON_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutcommon)
install(FILES ${NUTWIRELESS_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutwireless)
install(FILES ${NUTCLIENT_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutclient)
