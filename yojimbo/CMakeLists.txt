cmake_minimum_required(VERSION 3.1.0)

if(MSVC)
	add_custom_command(
	  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo/bin/x64/Release/yojimbo.lib
	  COMMAND msbuild /p:Configuration=Release Yojimbo.sln
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo
	)
	add_custom_target(
	  yojimbo_target
	  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo/bin/x64/Release/yojimbo.lib
	)
	add_library(yojimbo STATIC IMPORTED GLOBAL)
	set_target_properties(yojimbo
	  PROPERTIES
	  IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo/bin/x64/Release/yojimbo.lib
	  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo
	)
elseif(UNIX)
	add_custom_command(
	  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo/bin/libyojimbo.a
	  COMMAND make config=release_x64
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo
	)
	add_custom_target(
	  yojimbo_target
	  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo/bin/libyojimbo.a
	)
	add_library(yojimbo STATIC IMPORTED GLOBAL)
	set_target_properties(yojimbo
	  PROPERTIES
	  IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo/bin/libyojimbo.a
	  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo
	)
endif()