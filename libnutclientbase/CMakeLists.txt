# nutclientbase is linked by cnut; it uses only the "native" dbus interfaces,
# not the state tracking wrappers
add_library(nutclientbase OBJECT
	dbus.cpp
	dbus.h
)

object_link_libraries(nutclientbase PUBLIC
	nutcommon
	Qt5::Core
	Qt5::DBus
)

file(GLOB NUTCLIENT_BASE_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

install(FILES ${NUTCLIENT_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutclientbase)
