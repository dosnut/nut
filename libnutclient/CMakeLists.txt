project(nutclient)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

set(SOURCES_BASIC_MOC_H
	dbus.h
	)
set(SOURCSE_BASIC_CPP
	dbus.cpp
	)

set(SOURCES_MOC_H
	cinterface.h
	cenvironment.h
	cdevice.h
	cdevicemanager.h
	clog.h
	cnutservice.h
)
set(SOURCES_CPP
	cinterface.cpp
	cenvironment.cpp
	cdevice.cpp
	cdevicemanager.cpp
	clibnut.cpp
	clog.cpp
	cnutservice.cpp
)

set(SOURCES_OBJECT)
if(USE_WIRELESS)
	find_library(LIB_IW iw)
	set(SOURCES_OBJECT $<TARGET_OBJECTS:nutwireless>)
else(USE_WIRELESS)
	set(LIB_IW)
endif(USE_WIRELESS)

file(GLOB TRANSLATIONS_TS translations/${PROJECT_NAME}_*.ts)

qt4_wrap_cpp(SOURCES_BASIC_MOC_CPP ${SOURCES_BASIC_MOC_H})
qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})
my_qt4_create_translation(TRANSLATIONS_QM ${SOURCES_CPP} ${TRANSLATIONS_TS})

# nutclient_basic is linked by cnut; it uses only the "native" dbus interfaces,
# not the state tracking wrappers
add_library(${PROJECT_NAME}_basic OBJECT ${SOURCSE_BASIC_CPP} ${SOURCES_BASIC_MOC_CPP})

add_library(${PROJECT_NAME} SHARED ${SOURCES_CPP} ${SOURCES_MOC_CPP} ${SOURCES_OBJECT} ${TRANSLATIONS_QM} $<TARGET_OBJECTS:${PROJECT_NAME}_basic>)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION ${PACKAGE_SO_VERSION})

target_link_libraries(${PROJECT_NAME}
	nutcommon
	${LIB_NL}
	${LIB_IW}
	${QT_LIBRARIES}
)

file(GLOB NUTCLIENT_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${TRANSLATIONS_QM} DESTINATION ${INSTALL_SHARE_TARGET_PREFIX}/lang)
install(FILES ${NUTCLIENT_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutclient)
