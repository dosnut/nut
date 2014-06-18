project(nutcommon)

include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

set(SOURCES_MOC_H
	dbusmanager.h
)
set(SOURCES_CPP
	macaddress.cpp
	config.cpp
	common.cpp
	device.cpp
	dbusmanager.cpp
)

qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})

# recompile only a single file on version changes
add_library(${PROJECT_NAME}_version OBJECT version.cpp)
set_target_properties(${PROJECT_NAME}_version PROPERTIES COMPILE_FLAGS "-DNUT_VERSION='\"${PACKAGE_VERSION}\"'")

add_library(${PROJECT_NAME} SHARED ${SOURCES_CPP} ${SOURCES_MOC_CPP} $<TARGET_OBJECTS:${PROJECT_NAME}_version>)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION ${PACKAGE_SO_VERSION})

target_link_libraries(${PROJECT_NAME}
	${QT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

file(GLOB NUTCOMMON_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${NUTCOMMON_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutcommon)
