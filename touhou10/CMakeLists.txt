cmake_minimum_required(VERSION 3.7)
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
# See .vscode/launch.json and .vscode/settings.json if using TEMP_USE_CUSTOM_SDL
# 
if(NOT DEFINED TEMP_USE_CUSTOM_SDL)
	set(TEMP_USE_CUSTOM_SDL OFF)
else()
	MESSAGE(STATUS "INFO: TEMP_USE_CUSTOM_SDL is ${TEMP_USE_CUSTOM_SDL}")
endif()

if(NOT TEMP_USE_CUSTOM_SDL)
	find_package(SDL2 REQUIRED)
	MESSAGE(STATUS "INFO: SDL2_INCLUDE_DIRS is ${SDL2_INCLUDE_DIRS}")
	MESSAGE(STATUS "INFO: SDL2_LIBRARIES is ${SDL2_LIBRARIES}")

	include_directories(touhou10 ${SDL2_INCLUDE_DIRS})
else()
	include_directories(touhou10 ~/source/repos/SDL-2.30.6/include)
	include_directories(touhou10 ~/source/repos/SDL_mixer-2.8.0/include)
endif()

include_directories(touhou10 ../external/glad/include)
include_directories(touhou10 ../external/glm/include)
include_directories(touhou10 ../external/minicoro/include)
include_directories(touhou10 ../external/qoi/include)
include_directories(touhou10 ../external/stb/include)

add_executable(touhou10
	src/animation.cpp
	src/anim_data_boss_spellcard_anim.cpp
	src/assets.cpp
	src/boss_and_stage_data.cpp
	src/boss.cpp
	src/character_data.cpp
	src/console.cpp
	src/game.cpp
	src/main.cpp
	src/objects.cpp
	src/package.cpp
	src/particle_system.cpp
	src/player.cpp
	src/renderer.cpp
	src/shader_data.cpp
	src/sprite_data.cpp
	src/stage_0_data.cpp
	src/stage_1_data.cpp
	src/title_screen.cpp
	src/world.cpp)

if(NOT TEMP_USE_CUSTOM_SDL)
	target_link_libraries(touhou10 ${SDL2_LIBRARIES})
	target_link_libraries(touhou10 SDL2_mixer)
else()
	target_link_libraries(touhou10 "-L ~/source/repos/SDL-2.30.6/build      -lSDL2")
	target_link_libraries(touhou10 "-L ~/source/repos/SDL_mixer-2.8.0/build -lSDL2_mixer")
endif()

target_precompile_headers(touhou10 PRIVATE src/stdafx.h)
