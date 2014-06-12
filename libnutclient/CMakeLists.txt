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

set(SOURCES_OBJECT)
if(USE_WIRELESS)
	find_library(LIB_IW iw)
	set(SOURCES_OBJECT $<TARGET_OBJECTS:nutwireless>)
else(USE_WIRELESS)
	set(LIB_IW)
endif(USE_WIRELESS)


qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})

add_definitions(-DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX)

add_library(${PROJECT_NAME} SHARED ${SOURCES_CPP} ${SOURCES_MOC_CPP} ${SOURCES_OBJECT})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION 0)

target_link_libraries(${PROJECT_NAME}
	nutcommon
	${LIB_NL}
	${LIB_IW}
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
