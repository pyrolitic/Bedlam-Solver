
CXX = g++
OUT = cube-solver

debug:
	$(CXX) src/main.cpp src/solver.cpp -std=c++11 -g3 -O0 -o $(OUT)

optimized:
	$(CXX) src/main.cpp src/solver.cpp -std=c++11 -g0 -O3 -march=native -o $(OUT)

all:
	.debug 
