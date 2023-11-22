cmake_minimum_required(VERSION 3.25)
project(roamsdl)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${SDL2_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

add_executable(roamsdl Main.cpp Utility.h Utility.cpp Landscape.h Landscape.cpp Patch.h Patch.cpp
        App.cpp
        App.h
        Roam.cpp
        Roam.h)

target_link_libraries(roamsdl ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})
