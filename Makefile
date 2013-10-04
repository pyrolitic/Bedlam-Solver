
CC = gcc
CCDEPMODE = depmode=gcc3
CFLAGS = -g -O2
CPP = gcc -E
CPPFLAGS = 
CXX = g++
CXXFLAGS = -g3 -O0 -std=c++11
OUT = cube-solver

all: 
	$(CXX) src/main.cpp src/solver.cpp $(CXXFLAGS) -o $(OUT)
