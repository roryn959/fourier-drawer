cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(fourier_drawer)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES src/*.cpp)

add_executable(main ${SOURCES})

target_link_libraries(main ${SDL2_LIBRARIES})