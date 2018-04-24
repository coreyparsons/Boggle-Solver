#ifndef BOGGLE_H
#define BOGGLE_H

#include "main.h"
#include "threads.h"

struct BoardPos
{
	uint down, across;
	bool directionsBlocked[8];
};

struct Solution
{
	uint score;
	char** words;
	void* userData;
};

struct SolveThreadReadData
{
	int width;
	int height;
	void* dictRoot;
	char* board;
};

struct SolveThreadData
{
	SolveThreadReadData* solveThreadReadData;
	int rootDownStart;
	int rootAcrossStart;
	int nodesToComplete;
};

struct MemoryArena;
Solution* solve(char* board, int width, int height, WorkQueue* queue, MemoryArena* calcMemory, void* dictRoot, MemoryArena* outputMemory);

#endif