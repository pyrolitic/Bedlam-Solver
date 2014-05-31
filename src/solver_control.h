#ifndef SOLVER_CONTROL_H
#define SOLVER_CONTROL_H

#include "piece.h"
#include "timer.h"

#include <list>
#include <string>
#include <thread>
#include <mutex>

class SolverControl {
public:
	struct solution{
		struct piecePosition{
			int px, py, pz;
			int side;
		};

		piecePosition* positions;

		solution(int pieces){
			positions = (piecePosition*) malloc(pieces * sizeof(piecePosition));
		}

		~solution(){
			free(positions);
		}
	};

	SolverControl(vec3i worldSize, const std::list<Piece>& pieces){
		solverThread = std::thread(threadEntry, worldSize, pieces, this);
	}

	~SolverControl() {
		free(currentSolution);
	}

	void killThread(){

	}

	//for the solver thread
	void addSolution(const solution* sol){
		solutionAccess.lock();
		solutions.push_back(sol);
		solutionAccess.unlock();
	}

	int getSolutionCount(){
		solutionAccess.lock();
		int count = solutions.size();
		solutionAccess.unlock();
		return count;
	}

	const solution& getSolution(int i) const{
		return solutions[i];
	}

private:
	//once a solution is places here, it will not be touched by the solver thread again
	std::vector<solution*> solutions;
	std::thread solverThread;
	std::lock solutionAccess;

	static void threadEntry(vec3i worldSize, const std::list<Piece>& pieces, SolverControl& control){

	}
};

#endif