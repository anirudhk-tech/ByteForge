cmake_minimum_required(VERSION 3.16)
project(byteforge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
