mkdir -p ../out
g++ -g -D_DEBUG -o ../out/tools src/*.cpp \
	-I../touhou10/src -I../external/glad/include -I../external/glm/include -I../external/minicoro/include -I../external/qoi/include -I../external/stb/include \
	`sdl2-config --cflags --libs` -lSDL2_image
