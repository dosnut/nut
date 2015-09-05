add_executable(cnut
	$<TARGET_OBJECTS:nutclientbase>

	cnut_commands.cpp
	cnut_parsers.cpp
	main.cpp
)

target_link_object(cnut
	nutclientbase
)

install(TARGETS cnut DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
