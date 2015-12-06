# TARGET_OBJECTS and CMAKE_AUTOMOC are buggy with ninja in cmake < 3.4
set(CMAKE_AUTOMOC OFF)

add_executable(cnut
	$<TARGET_OBJECTS:nutclientbase>

	cnut_commands.cpp
	cnut_commands.h
	cnut_parsers.cpp
	cnut_parsers.h
	cnut_types.h
	main.cpp
)

target_link_object(cnut
	nutclientbase
)

install(TARGETS cnut DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
