mkdir build
emcc -o build/index.html^
 src/animation.cpp^
 src/anim_data_boss_spellcard_anim.cpp^
 src/assets.cpp^
 src/boss_and_stage_data.cpp^
 src/boss.cpp^
 src/character_data.cpp^
 src/console.cpp^
 src/font_data.cpp^
 src/game.cpp^
 src/main.cpp^
 src/menu.cpp^
 src/objects.cpp^
 src/package.cpp^
 src/particle_system.cpp^
 src/player.cpp^
 src/renderer.cpp^
 src/sprite_data.cpp^
 src/stage_0_data.cpp^
 src/stage_1_data.cpp^
 src/title_screen.cpp^
 src/world.cpp^
 -I../external/glad/include^
 -I../external/glm/include^
 -I../external/minicoro/include^
 -I../external/qoi/include^
 -I../external/stb/include^
 -DTH_DEBUG=0 -DNDEBUG -O3^
 -sUSE_SDL=2 -sUSE_SDL_MIXER=2 -sWASM=1 -sASYNCIFY=1 -sUSE_WEBGL2=1 -sASSERTIONS=1^
 --preload-file fonts^
 --preload-file models^
 --preload-file music^
 --preload-file shaders^
 --preload-file sounds^
 --preload-file textures
