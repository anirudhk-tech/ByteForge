cmake_minimum_required(VERSION 3.16)
project(byteforge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)
if(IPO_SUPPORTED)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

file(GLOB LIB_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
add_library(byteforge ${LIB_SOURCES})

target_include_directories(byteforge PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp"
)

foreach(src ${EXAMPLE_SOURCES})
  get_filename_component(name ${src} NAME_WE) 
  add_executable(${name} ${src})
  target_link_libraries(${name} PRIVATE byteforge)
endforeach()
