cmake_minimum_required(VERSION 3.23)

project(aZeroInput)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")

option(COMPILE_EXAMPLE "Make it as an exe instead of lib" OFF)

include(FetchContent)

if (COMPILE_EXAMPLE)
    add_executable(aZeroInput
        "src/aZeroInput.cpp" 
        "examples/main.cpp")

    FetchContent_Declare(
        aZeroWindow
        GIT_REPOSITORY https://github.com/NoahSch1999/aZero-Window.git
        GIT_TAG main
    )
    set(COMPILE_EXAMPLE OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(aZeroWindow)

    target_link_libraries(aZeroInput PUBLIC aZeroWindow)
    target_compile_definitions(aZeroInput PUBLIC COMPILE_EXAMPLE)
else()
    add_library(aZeroInput 
        "src/aZeroInput.cpp")

    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.x
    )

    FetchContent_MakeAvailable(SDL3)
    target_link_libraries(aZeroInput PUBLIC SDL3::SDL3)
endif()

target_include_directories(aZeroInput
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

