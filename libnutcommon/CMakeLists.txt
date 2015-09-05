# recompile only a single file on version changes
add_library(nutcommon_version OBJECT
	version.cpp
)
set_target_properties(nutcommon_version PROPERTIES COMPILE_FLAGS "-DNUT_VERSION='\"${PACKAGE_VERSION}\"'")
object_link_libraries(nutcommon_version PUBLIC
	Qt5::Core
)

add_library(nutcommon SHARED
	common.cpp
	config.cpp
	dbusmanager.cpp
	device.cpp
	macaddress.cpp

	$<TARGET_OBJECTS:nutcommon_version>
)
set_target_properties(nutcommon PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION ${PACKAGE_SO_VERSION})

target_link_object(nutcommon
	nutcommon_version
	Qt5::DBus
	Qt5::Network
)

install(TARGETS nutcommon LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

file(GLOB NUTCOMMON_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${NUTCOMMON_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutcommon)
