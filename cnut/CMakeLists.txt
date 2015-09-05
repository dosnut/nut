project(cnut)

include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

set(SOURCES_CPP main.cpp
	cnut_commands.cpp
	cnut_parsers.cpp
)

# only link the "basic" (dbus.cpp) part of libnutclient as "object"
add_executable(${PROJECT_NAME}
	${SOURCES_CPP}
	$<TARGET_OBJECTS:nutclient_basic>
)

target_link_libraries(${PROJECT_NAME}
	nutcommon
	${QT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
