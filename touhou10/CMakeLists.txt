cmake_minimum_required(VERSION 3.20)
project(touhou10)

# 
# On Windows, use visual studio
# On Linux and Mac, use Cmake
# 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

add_compile_definitions(DEVELOPER)

if(APPLE)
	add_compile_definitions(_THREAD_SAFE)
elseif(UNIX)
	add_compile_definitions(_REENTRANT)
endif()

include_directories(${PROJECT_NAME} ../external/glad/include)
include_directories(${PROJECT_NAME} ../external/glm/include)
include_directories(${PROJECT_NAME} ../external/minicoro/include)
include_directories(${PROJECT_NAME} ../external/qoi/include)
include_directories(${PROJECT_NAME} ../external/stb/include)

# 
# On Mac, homebrew seems to install SDL2_mixer into /usr/local/include/SDL2 and /usr/local/Cellar/sdl2_mixer/2.8.0
# The /usr/local/include/SDL2 directory also contains SDL2, but find_package(SDL2)
# returns /usr/local/Cellar/sdl2_mixer/2.8.0, which doesn't.
# And Cmake (or SDL) doesn't ship with FindSDL2_mixer.cmake, so we can't use find_package(SDL2_mixer).
# 
# So, just manually check in /usr/include and /usr/local/include.
# 
# You can customize theese variables to use a custom build of SDL.
# See .vscode/settings.json for an example (don't forget about LD_LIBRARY_PATH).
# 
if(NOT SDL2_INCLUDE_DIRS AND NOT SDL2_LIBRARIES AND NOT SDL2_MIXER_INCLUDE_DIRS AND NOT SDL2_MIXER_LIBRARIES)
	if(EXISTS /usr/include/SDL2/SDL.h)
		MESSAGE(STATUS "INFO: Found SDL2 in /usr/include/SDL2.")
		set(SDL2_INCLUDE_DIRS /usr/include/SDL2)
		set(SDL2_LIBRARIES SDL2)
	elseif(EXISTS /usr/local/include/SDL2/SDL.h)
		MESSAGE(STATUS "INFO: Found SDL2 in /usr/local/include/SDL2.")
		set(SDL2_INCLUDE_DIRS /usr/local/include/SDL2)
		set(SDL2_LIBRARIES "-L/usr/local/lib -lSDL2") # Mac
	else()
		MESSAGE(FATAL_ERROR "FATAL: Couldn't find SDL2.")
	endif()

	if(EXISTS /usr/include/SDL2/SDL_mixer.h)
		MESSAGE(STATUS "INFO: Found SDL2_mixer in /usr/include/SDL2.")
		set(SDL2_MIXER_INCLUDE_DIRS /usr/include/SDL2)
		set(SDL2_MIXER_LIBRARIES SDL2_mixer)
	elseif(EXISTS /usr/local/include/SDL2/SDL_mixer.h)
		MESSAGE(STATUS "INFO: Found SDL2_mixer in /usr/local/include/SDL2.")
		set(SDL2_MIXER_INCLUDE_DIRS /usr/local/include/SDL2)
		set(SDL2_MIXER_LIBRARIES "-L/usr/local/lib -lSDL2_mixer") # Mac
	else()
		MESSAGE(FATAL_ERROR "FATAL: Couldn't find SDL2_mixer.")
	endif()
endif()

MESSAGE(STATUS "INFO: SDL2_INCLUDE_DIRS is ${SDL2_INCLUDE_DIRS}")
include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})

MESSAGE(STATUS "INFO: SDL2_MIXER_INCLUDE_DIRS is ${SDL2_MIXER_INCLUDE_DIRS}")
include_directories(${PROJECT_NAME} ${SDL2_MIXER_INCLUDE_DIRS})

set(SOURCES
	src/animation.cpp
	src/anim_data_boss_spellcard_anim.cpp
	src/assets.cpp
	src/boss_and_stage_data.cpp
	src/boss.cpp
	src/character_data.cpp
	src/console.cpp
	src/font.cpp
	src/font_data.cpp
	src/game.cpp
	src/main.cpp
	src/menu.cpp
	src/objects.cpp
	src/package.cpp
	src/particle_system.cpp
	src/player.cpp
	src/renderer.cpp
	src/single_header.cpp
	src/sprite_data.cpp
	src/sprite.cpp
	src/stage_0_data.cpp
	src/stage_1_data.cpp
	src/stdafx.cpp
	src/texture.cpp
	src/title_screen.cpp
	src/window_creation.cpp
	src/world.cpp
	src/util.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

MESSAGE(STATUS "INFO: SDL2_LIBRARIES is ${SDL2_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

MESSAGE(STATUS "INFO: SDL2_MIXER_LIBRARIES is ${SDL2_MIXER_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} ${SDL2_MIXER_LIBRARIES})

target_precompile_headers(${PROJECT_NAME} PRIVATE src/stdafx.h)
