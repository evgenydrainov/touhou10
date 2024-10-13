cmake_minimum_required(VERSION 3.20)
project(touhou10)

# 
# I've only tested this file on Linux.
# On Windows, use the Visual Studio solution.
# 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(TH_DEBUG=$<CONFIG:Debug>)
add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)
add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

# 
# Include stuff.
# 
include_directories(${PROJECT_NAME} ../external/glad/include)
include_directories(${PROJECT_NAME} ../external/glm/include)
include_directories(${PROJECT_NAME} ../external/minicoro/include)
include_directories(${PROJECT_NAME} ../external/qoi/include)
include_directories(${PROJECT_NAME} ../external/stb/include)

# 
# You can set SDL2_INCLUDE_DIRS, SDL2_LIBRARIES, SDL2_MIXER_INCLUDE_DIRS and SDL2_MIXER_LIBRARIES.
# If you change SDL2_LIBRARIES, you'll probably also want to setup LD_LIBRARY_PATH.
# See .vscode/
# 
if(NOT SDL2_INCLUDE_DIRS)
	find_package(SDL2 REQUIRED)
endif()

MESSAGE(STATUS "INFO: SDL2_INCLUDE_DIRS is ${SDL2_INCLUDE_DIRS}")
include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})

if(SDL2_MIXER_INCLUDE_DIRS)
	MESSAGE(STATUS "INFO: SDL2_MIXER_INCLUDE_DIRS is ${SDL2_MIXER_INCLUDE_DIRS}")
	include_directories(${PROJECT_NAME} ${SDL2_MIXER_INCLUDE_DIRS})
endif()

set(SOURCES
	src/animation.cpp
	src/anim_data_boss_spellcard_anim.cpp
	src/assets.cpp
	src/boss_and_stage_data.cpp
	src/boss.cpp
	src/character_data.cpp
	src/console.cpp
	src/font_data.cpp
	src/game.cpp
	src/main.cpp
	src/menu.cpp
	src/objects.cpp
	src/package.cpp
	src/particle_system.cpp
	src/player.cpp
	src/renderer.cpp
	src/sprite_data.cpp
	src/stage_0_data.cpp
	src/stage_1_data.cpp
	src/title_screen.cpp
	src/world.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

MESSAGE(STATUS "INFO: SDL2_LIBRARIES is ${SDL2_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

if(SDL2_MIXER_LIBRARIES)
	MESSAGE(STATUS "INFO: SDL2_MIXER_LIBRARIES is ${SDL2_MIXER_LIBRARIES}")
	target_link_libraries(${PROJECT_NAME} ${SDL2_MIXER_LIBRARIES})
else()
	target_link_libraries(${PROJECT_NAME} SDL2_mixer)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/stdafx.h)
