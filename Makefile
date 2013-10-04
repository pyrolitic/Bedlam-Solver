
CC = gcc
CCDEPMODE = depmode=gcc3
CFLAGS = -g -O2
CPP = gcc -E
CPPFLAGS = 
CXX = g++
CXXFLAGS = -g3 -O0

.all: $(CXX) src/draw.cpp src/image.cpp src/main.cpp image/solver.cpp $(CXXFLAGS)
