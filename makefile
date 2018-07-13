all:
	g++ main.cpp -o bin/main -std=c++11 -Wall -Iinclude -lglfw -lGLEW -lGLU -lGL
