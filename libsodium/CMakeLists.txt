cmake_minimum_required(VERSION 3.1.0)

if(MSVC)
	add_custom_target(
	  sodium_target
	  COMMAND msbuild /p:Configuration=StaticRelease /p:Platform=x64 builds/msvc/vs2017/libsodium.sln
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libsodium
	  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/bin/x64/Release/v141/static/libsodium.lib
	)
	add_library(sodium STATIC IMPORTED GLOBAL)
	add_dependencies(sodium sodium_target)
	set_target_properties(sodium
	  PROPERTIES
	  IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/bin/x64/Release/v141/static/libsodium.lib
	  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/libsodium
	)
elseif(UNIX)
	add_custom_command(
	  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/.libs/libsodium.so
	  COMMAND ./autogen.sh && ./configure && make
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libsodium
	)
	add_custom_target(
	  sodium_target
	  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/.libs/libsodium.so
	)
	add_library(sodium SHARED IMPORTED GLOBAL)
	add_dependencies(sodium sodium_target)
	set_target_properties(sodium
	  PROPERTIES
	  IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/.libs/libsodium.so
	  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/libsodium
	)
endif()