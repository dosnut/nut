file(GLOB TRANSLATIONS_TS translations/nutclient_*.ts)
file(GLOB TRANSLATIONS_USAGES *.cpp)
qt5_create_translation(TRANSLATIONS_QM ${TRANSLATIONS_USAGES} ${TRANSLATIONS_TS})

add_library(nutclient SHARED
	cdevice.cpp
	cdevicemanager.cpp
	cenvironment.cpp
	cinterface.cpp
	clibnut.cpp
	clog.cpp
	cnutservice.cpp

	${TRANSLATIONS_QM}

	$<TARGET_OBJECTS:nutwireless>
	$<TARGET_OBJECTS:nutclientbase>
)
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
