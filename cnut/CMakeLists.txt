project(cnut)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 COMPONENTS QtCore QtNetwork QtDBus REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

add_definitions(-D_REENTRANT)

set(SOURCES_MOC_H
	server_proxy.h
)
set(SOURCES_CPP main.cpp
	cnut_commands.cpp
	cnut_parsers.cpp
	server_proxy.cpp
)

qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})

add_executable(${PROJECT_NAME}
	${SOURCES_CPP}
	${SOURCES_MOC_CPP}
)

target_link_libraries(${PROJECT_NAME}
	${QT_QTCORE_LIBRARY}
	${QT_QTNETWORK_LIBRARY}
	${QT_QTDBUS_LIBRARY}
	${CMAKE_BINARY_DIR}/libnutcommon/libnutcommon.a
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
