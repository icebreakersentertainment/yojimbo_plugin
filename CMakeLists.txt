cmake_minimum_required(VERSION 3.1.0)

project(yojimbo_plugin)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(YOJIMBO_PLUGIN_COMPILER_FLAGS "")
if(MSVC)
  set(YOJIMBO_PLUGIN_COMPILER_FLAGS "/EHsc /MP")
  if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(YOJIMBO_PLUGIN_COMPILER_FLAGS "/EHsc /MP")
  endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(yojimbo)
add_subdirectory(libsodium)

option(ENABLE_TESTING "Build mbed TLS tests." OFF)
add_subdirectory(mbedtls/mbedtls)

# Headers
include_directories(include)

# Source
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(yojimbo_plugin SHARED ${SOURCES})

target_include_directories(yojimbo_plugin PRIVATE ${ICEENGINE_INCLUDE_DIRS})
target_include_directories(yojimbo_plugin PRIVATE ${BOOST_INCLUDE_DIRS})
target_include_directories(yojimbo_plugin PRIVATE ${GLM_INCLUDE_DIRS})

target_compile_options(yojimbo_plugin PRIVATE ${YOJIMBO_PLUGIN_COMPILER_FLAGS})

target_link_libraries(yojimbo_plugin yojimbo)
target_link_libraries(yojimbo_plugin sodium)
target_link_libraries(yojimbo_plugin mbedtls)
target_link_libraries(yojimbo_plugin mbedx509)
target_link_libraries(yojimbo_plugin mbedcrypto)