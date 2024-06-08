mkdir -p ../out
g++ -g -D_DEBUG -Wall -Wno-unused-function -Wno-switch -o ../out/touhou10 src/*.cpp \
	-I../external/glad/include -I../external/glm/include -I../external/minicoro/include -I../external/qoi/include -I../external/stb/include \
	`sdl2-config --cflags --libs`
