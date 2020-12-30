cmake_minimum_required(VERSION 3.1.0)

find_package(MbedTLS REQUIRED)
find_package(libsodium REQUIRED)

set(OUTPUT_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo/bin/libyojimbo.a)
set(BUILD_COMMAND bash ../compile.sh ${MbedTLS_INCLUDE_DIRS} ${libsodium_INCLUDE_DIRS} ${libsodium_LIB_DIRS} ${MbedTLS_LIB_DIRS})

if(MSVC)
  set(OUTPUT_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo/bin/x64/Release/yojimbo.lib)
  set(BUILD_COMMAND msbuild /p:Configuration=Release Yojimbo.sln)
endif()

add_custom_command(
  OUTPUT ${OUTPUT_LIBRARY}
  COMMAND ${BUILD_COMMAND}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo
)
add_custom_target(
  yojimbo_target
  DEPENDS ${OUTPUT_LIBRARY}
)
add_library(yojimbo STATIC IMPORTED GLOBAL)
add_dependencies(yojimbo yojimbo_target)
set_target_properties(yojimbo
  PROPERTIES
  IMPORTED_LOCATION ${OUTPUT_LIBRARY}
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/yojimbo
)
