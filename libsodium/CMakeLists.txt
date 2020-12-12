cmake_minimum_required(VERSION 3.1.0)

set(OUTPUT_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/.libs/libsodium.so)
set(BUILD_COMMAND ./autogen.sh && ./configure && make)

if(MSVC)
  set(OUTPUT_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/bin/x64/Release/v142/static/libsodium.lib)
  set(BUILD_COMMAND msbuild /p:Configuration=StaticRelease /p:Platform=x64 builds/msvc/vs2019/libsodium.sln)
elseif(APPLE)
  set(OUTPUT_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/.libs/libsodium.a)
endif()

add_custom_command(
  OUTPUT ${OUTPUT_LIBRARY}
  COMMAND ${BUILD_COMMAND}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libsodium
)
add_custom_target(
  sodium_target
  DEPENDS ${OUTPUT_LIBRARY}
)
add_library(sodium STATIC IMPORTED GLOBAL)
add_dependencies(sodium sodium_target)
set_target_properties(sodium
  PROPERTIES
  IMPORTED_LOCATION ${OUTPUT_LIBRARY}
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/libsodium
)
