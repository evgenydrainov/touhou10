cmake_minimum_required(VERSION 3.7)
project(touhou10)

set(CMAKE_BUILD_TYPE Debug)
add_compile_definitions(_DEBUG)
add_compile_definitions(TH_DEBUG=1)

find_package(SDL2 REQUIRED)
include_directories(touhou10 ${SDL2_INCLUDE_DIRS})

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
	src/stdafx.cpp
	src/title_screen.cpp
	src/world.cpp)
target_link_libraries(touhou10 ${SDL2_LIBRARIES})
target_link_libraries(touhou10 SDL2_mixer)
