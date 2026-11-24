cmake_minimum_required(VERSION 3.24)
project(PocketPlaces LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

if(ANDROID)
    add_library(PocketPlaces SHARED src/main.cpp)
    target_link_libraries(PocketPlaces PRIVATE raylib log android EGL GLESv2 OpenSLES)
    target_compile_definitions(PocketPlaces PRIVATE PLATFORM_ANDROID)
else()
    add_executable(PocketPlaces src/main.cpp)
    target_link_libraries(PocketPlaces PRIVATE raylib)
endif()

target_include_directories(PocketPlaces PRIVATE ${raylib_SOURCE_DIR}/src)
target_compile_options(PocketPlaces PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
