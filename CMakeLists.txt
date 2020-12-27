cmake_minimum_required(VERSION 3.1.0)

project(yojimbo_plugin)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(YOJIMBO_PLUGIN_COMPILER_FLAGS "")
set(YOJIMBO_PLUGIN_DEFINITIONS "")
if(MSVC)
  list(APPEND YOJIMBO_PLUGIN_COMPILER_FLAGS /EHsc /MP)
  if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    list(APPEND YOJIMBO_PLUGIN_COMPILER_FLAGS /EHsc /MP)
  endif()
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo OR ICEENGINE_ENABLE_DEBUG_LOGGING)
  list(APPEND YOJIMBO_PLUGIN_DEFINITIONS -DICEENGINE_ENABLE_DEBUG_LOGGING)
endif()

if(ICEENGINE_ENABLE_TRACE_LOGGING)
  list(APPEND YOJIMBO_PLUGIN_DEFINITIONS -DICEENGINE_ENABLE_TRACE_LOGGING)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(yojimbo)
add_subdirectory(libsodium)

option(ENABLE_TESTING "Build mbed TLS tests." OFF)
add_subdirectory(mbedtls/mbedtls)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(glm REQUIRED)
find_package(Boost REQUIRED)

# Source
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(yojimbo_plugin SHARED ${SOURCES})

# Headers
target_include_directories(yojimbo_plugin PRIVATE include)

target_include_directories(yojimbo_plugin PRIVATE ${ICEENGINE_INCLUDE_DIRS})
target_include_directories(yojimbo_plugin PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_definitions(yojimbo_plugin PRIVATE ${YOJIMBO_PLUGIN_DEFINITIONS})
target_compile_options(yojimbo_plugin PRIVATE ${YOJIMBO_PLUGIN_COMPILER_FLAGS})

target_link_libraries(yojimbo_plugin PRIVATE yojimbo)
target_link_libraries(yojimbo_plugin PRIVATE sodium)
target_link_libraries(yojimbo_plugin PRIVATE mbedtls)
target_link_libraries(yojimbo_plugin PRIVATE mbedx509)
target_link_libraries(yojimbo_plugin PRIVATE mbedcrypto)
target_link_libraries(yojimbo_plugin PRIVATE glm::glm)
