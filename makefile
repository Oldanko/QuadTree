all:
	g++ main.cpp -o bin/main -std=c++17 -Wall -Iinclude -lglfw -lGLEW -lGLU -lGL
