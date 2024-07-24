mkdir -p ../out
g++ \
	-g -std=c++17 \
	-DTH_DEBUG=1 -D_DEBUG \
	-o ../out/tools \
	src/*.cpp \
	-I../external/glad/include \
	-I../external/glm/include \
	-I../external/minicoro/include \
	-I../external/qoi/include \
	-I../external/stb/include \
	`sdl2-config --cflags --libs` \
	-lSDL2_image
