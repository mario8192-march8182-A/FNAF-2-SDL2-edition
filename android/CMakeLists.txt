cmake_minimum_required(VERSION 3.10)
project(FNAF2_Android)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Android specific settings
set(CMAKE_ANDROID_PLATFORM android-21)
set(CMAKE_ANDROID_STL_TYPE c++_shared)

# Find SDL2 and its extensions
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_net REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
include_directories(${SDL2_NET_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(GAME_SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/game.c
    ${CMAKE_SOURCE_DIR}/src/renderer.c
    ${CMAKE_SOURCE_DIR}/src/audio.c
    ${CMAKE_SOURCE_DIR}/src/input.c
    ${CMAKE_SOURCE_DIR}/src/camera.c
    ${CMAKE_SOURCE_DIR}/src/entity.c
    ${CMAKE_SOURCE_DIR}/src/assets.c
    ${CMAKE_SOURCE_DIR}/src/ai_system.c
)

# Create shared library for Android
add_library(fnaf2-core SHARED ${GAME_SOURCES})

# Link SDL2 libraries
target_link_libraries(fnaf2-core
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_NET_LIBRARIES}
    m
    log
)

# Android-specific flags
if(ANDROID)
    target_compile_options(fnaf2-core PRIVATE -O3 -fPIC)
    set_target_properties(fnaf2-core PROPERTIES
        VERSION 1.0.0
        SOVERSION 1
    )
endif()
