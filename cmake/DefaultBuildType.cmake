if(NOT CMAKE_BUILD_TYPE)
	# default build type
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE RelWithDebInfo)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
