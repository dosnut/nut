find_library(LIB_IW iw)

if(NOT LIB_IW)
	message(FATAL_ERROR "couldn't find library iw")
endif()

if(NOT(TARGET "libiw"))
	add_library("libiw" INTERFACE)
	target_link_libraries("libiw" INTERFACE ${LIB_IW})
	target_include_directories("libiw" INTERFACE)
	target_compile_options("libiw" INTERFACE)
endif()
