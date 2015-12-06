include(CMakeParseArguments)

set(NUT_UPDATE_QT_TRANSLATIONS OFF CACHE BOOL "update translation files in source")

# mandatory parameters:
#   SOURCES       source files to find texts to translate in
#   TRANSLATIONS  translation files
# optional parameters:
#   TARGET        target to copy include directories from and add generated sources to
#                 if not set include directories are taken from current directory
#   VARIABLE      variable to store generated sources in
function(qt5_translate)
	set(options)
	set(oneValueArgs TARGET VARIABLE)
	set(multiValueArgs SOURCES TRANSLATIONS OPTIONS)

	cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if(ARG_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "qt5_translate: unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
	endif()
	if(NOT ARG_SOURCES OR NOT ARG_TRANSLATIONS)
		message(FATAL_ERROR "qt5_translate: missing mandatory parameters")
	endif()

	# hash all arguments for a unique stable identifier
	# make it change when NUT_UPDATE_QT_TRANSLATIONS is changed to trigger a new update
	string(SHA256 id "qt5_translate __id__ ${NUT_UPDATE_QT_TRANSLATIONS} ${ARGV}")

	# build list of source files and include paths
	set(_lst_file_srcs)
	foreach(_lst_file_src ${ARG_SOURCES})
		set(_lst_file_srcs "${_lst_file_src}\n${_lst_file_srcs}")
	endforeach()

	if(ARG_TARGET)
		get_target_property(_inc_DIRS "${ARG_TARGET}" INCLUDE_DIRECTORIES)
	else()
		get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)
	endif()
	foreach(_pro_include ${_inc_DIRS})
		get_filename_component(_abs_include "${_pro_include}" ABSOLUTE)
		set(_lst_file_srcs "-I${_pro_include}\n${_lst_file_srcs}")
	endforeach()

	set(_ts_sources_list_file "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${id}_sources_list")
	file(GENERATE
		OUTPUT "${_ts_sources_list_file}"
		CONTENT "${_lst_file_srcs}")

	set(_updated_translations)
	foreach(_ts_file ${ARG_TRANSLATIONS})
		file(RELATIVE_PATH _ts_file "${CMAKE_CURRENT_SOURCE_DIR}" "${_ts_file}")
		set(_target_ts_file "${CMAKE_CURRENT_BINARY_DIR}/${_ts_file}")
		list(APPEND _updated_translations "${_target_ts_file}")

		if(NUT_UPDATE_QT_TRANSLATIONS)
			add_custom_command(OUTPUT "${_target_ts_file}"
				COMMAND "${CMAKE_COMMAND}"
					ARGS -E copy "${_ts_file}" "${_target_ts_file}"
				COMMAND "${Qt5_LUPDATE_EXECUTABLE}"
					ARGS ${ARG_OPTIONS} "@${_ts_sources_list_file}" -ts "${_target_ts_file}"
				COMMAND "${CMAKE_COMMAND}" # copy back if changed
					ARGS -E copy_if_different "${_target_ts_file}" "${_ts_file}"
				DEPENDS ${ARG_SOURCES} "${_ts_sources_list_file}" "${_ts_file}"
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				VERBATIM)
		else()
			add_custom_command(OUTPUT "${_target_ts_file}"
				COMMAND "${CMAKE_COMMAND}"
					ARGS -E copy "${_ts_file}" "${_target_ts_file}"
				COMMAND "${Qt5_LUPDATE_EXECUTABLE}"
					ARGS ${ARG_OPTIONS} "@${_ts_sources_list_file}" -ts "${_target_ts_file}"
				DEPENDS ${ARG_SOURCES} "${_ts_sources_list_file}" "${_ts_file}"
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				VERBATIM)
		endif()
	endforeach()

	qt5_add_translation(_out_qm_files ${_updated_translations})
	if(ARG_VARIABLE)
		set(${ARG_VARIABLE} ${_out_qm_files} PARENT_SCOPE)
	endif()
	if(ARG_TARGET)
		target_sources(${ARG_TARGET} PRIVATE ${_out_qm_files})
	endif()
endfunction()
