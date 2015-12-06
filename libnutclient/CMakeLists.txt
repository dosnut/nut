file(GLOB TRANSLATIONS_TS translations/nutclient_*.ts)
file(GLOB TRANSLATIONS_USAGES *.cpp)

# TARGET_OBJECTS and CMAKE_AUTOMOC are buggy with ninja in cmake < 3.4
set(CMAKE_AUTOMOC OFF)

qt5_wrap_cpp(nutclient_MOC_SOURCE
	cdevice.h
	cdevicemanager.h
	cenvironment.h
	cinterface.h
#	clibnut.h
#	client.h
	clog.h
	cnutservice.h

	TARGET nutclient
	)

add_library(nutclient SHARED
	cdevice.cpp
	cdevice.h
	cdevicemanager.cpp
	cdevicemanager.h
	cenvironment.cpp
	cenvironment.h
	cinterface.cpp
	cinterface.h
	clibnut.cpp
	clibnut.h
	client.h
	clog.cpp
	clog.h
	cnutservice.cpp
	cnutservice.h

	${nutclient_MOC_SOURCE}

	$<TARGET_OBJECTS:nutwireless>
	$<TARGET_OBJECTS:nutclientbase>
)
qt5_translate(
	SOURCES ${TRANSLATIONS_USAGES}
	TRANSLATIONS ${TRANSLATIONS_TS}
	TARGET nutclient)

set_target_properties(nutclient PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION ${PACKAGE_SO_VERSION})


target_link_object(nutclient
	nutclientbase
)
if(USE_WIRELESS)
	target_link_object(nutclient
		nutwireless
	)
endif()
target_link_libraries(nutclient
	libnl
	libiw
)

file(GLOB NUTCLIENT_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

install(TARGETS nutclient LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${TRANSLATIONS_QM} DESTINATION ${INSTALL_SHARE_TARGET_PREFIX}/lang)
install(FILES ${NUTCLIENT_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutclient)
