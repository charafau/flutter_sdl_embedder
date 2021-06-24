cmake_minimum_required(VERSION 3.15)
project(embedder)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17" )
add_executable(embedder FlutterEmbedderSDL.cpp)

############################################################
# SDL
############################################################
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(embedder ${SDL2_LIBRARIES})

############################################################
# Flutter Engine
############################################################
# This is assuming you've built a local version of the Flutter Engine.  If you
# downloaded yours is from the internet you'll have to change this.

include_directories("/home/charafau/Utils/engine/src/flutter/shell/platform/common/public/")
include_directories("/home/charafau/Utils/engine/src/flutter/shell/platform/common/client_wrapper/include/flutter")

find_library(FLUTTER_LIB flutter_engine PATHS ${CMAKE_SOURCE_DIR}/)
target_link_libraries(embedder ${FLUTTER_LIB})
# Copy the flutter library here since the shared library
# name is `./libflutter_engine.dylib`.
add_custom_command(
        TARGET embedder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${FLUTTER_LIB}
                ${CMAKE_CURRENT_BINARY_DIR})
