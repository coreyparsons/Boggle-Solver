#include "boggle.h"
#include "utilities.h"

int dirFrom(int downDir, int acrossDir)
{
	if      (downDir == 0 && acrossDir == 1)   { return 0; }
	else if (downDir == 1 && acrossDir == 1)   { return 1; }
	else if (downDir == 1 && acrossDir == 0)   { return 2; }
	else if (downDir == 1 && acrossDir == -1)  { return 3; }
	else if (downDir == 0 && acrossDir == -1)  { return 4; }
	else if (downDir == -1 && acrossDir == -1) { return 5; }
	else if (downDir == -1 && acrossDir == 0)  { return 6; }
	else if (downDir == -1 && acrossDir == 1)  { return 7; }
	else { assert(0); return 0; }
}

void turnPoints(int &downDir, int &acrossDir)
{
	if      (downDir == 0 && acrossDir == 1)   { downDir = 1; acrossDir = 1; return; }
	else if (downDir == 1 && acrossDir == 1)   { downDir = 1; acrossDir = 0; return; }
	else if (downDir == 1 && acrossDir == 0)   { downDir = 1; acrossDir = -1; return; }
	else if (downDir == 1 && acrossDir == -1)  { downDir = 0; acrossDir = -1; return; }
	else if (downDir == 0 && acrossDir == -1)  { downDir = -1; acrossDir = -1; return; }
	else if (downDir == -1 && acrossDir == -1) { downDir = -1; acrossDir = 0; return; }
	else if (downDir == -1 && acrossDir == 0)  { downDir = -1; acrossDir = 1; return; }
	else if (downDir == -1 && acrossDir == 1)  { downDir = 0; acrossDir = 1;  return; }
	else { assert(0); }
}

void toDirs(int &dirDown, int &dirAcross, char dir)
{
	dirDown = ((bool)(dir & 3)) * (1 - ((dir & 4) >> 1));
	int dir2 = (dir + 10) % 8;
	dirAcross = ((bool)(dir2 & 3)) * (1 - ((dir2 & 4) >> 1));

	assert(dirDown == 1 || dirDown == 0 || dirDown == -1);
	assert(dirAcross == 1 || dirAcross == 0 || dirAcross == -1);
	assert(!((dirDown == 0) && (dirAcross == 0)));
}

void moveBackAndBlock(int &endDown, int &endAcross, BoardPos path[], uint8 &checkingWordIndex, char checkingWord[], char &dir)
{
	assert(path[checkingWordIndex].down == endDown);
	assert(path[checkingWordIndex].across == endAcross);

	int oldEndDown = endDown;
	int oldEndAcross = endAcross;

	int newEndDown = path[checkingWordIndex - 1].down;
	int newEndAcross = path[checkingWordIndex - 1].across;

	assert((oldEndAcross - newEndAcross) == 0 || (oldEndAcross - newEndAcross) == 1 || (oldEndAcross - newEndAcross) == -1);
	assert((oldEndDown - newEndDown) == 0 || (oldEndDown - newEndDown) == 1 || (oldEndDown - newEndDown) == -1);
	path[checkingWordIndex - 1].directionsBlocked[dirFrom((oldEndDown - newEndDown), (oldEndAcross - newEndAcross))] = true;

	endDown = newEndDown;
	endAcross = newEndAcross;

	path[checkingWordIndex] = {};
	checkingWord[checkingWordIndex] = 0;
	checkingWordIndex--;

	dir = 0;

	assert(path[checkingWordIndex].down == endDown);
	assert(path[checkingWordIndex].across == endAcross);
}

void markEdgeLocations(BoardPos* path, int endDown, int endAcross, int width, int height, uint checkingWordIndex)
{
	int checkEdgeDirDown = 0;
	int checkEdgeDirAcross = 1;

	if (!(endDown == 0 || endAcross == 0 || endDown == height - 1 || endAcross == width - 1))
	{
		return;
	}

	for (int i = 0; i < 8; i++)
	{
		int checkEdgeDown = endDown + checkEdgeDirDown;
		int checkEdgeAcross = endAcross + checkEdgeDirAcross;

		if (checkEdgeDown < 0 || checkEdgeDown >= height || checkEdgeAcross < 0 || checkEdgeAcross >= width)
		{
			path[checkingWordIndex].directionsBlocked[dirFrom(checkEdgeDirDown, checkEdgeDirAcross)] = true;
			assert(path[checkingWordIndex].down == endDown);
			assert(path[checkingWordIndex].across == endAcross);
		}
		turnPoints(checkEdgeDirDown, checkEdgeDirAcross);
	}
}

int to1d(int down, int across, int width)
{
	int result = (down * width) + across;
	return result;
}

int numSetBits(uint i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void progressNodes(char* word, uint& wordIndex, char* dictRoot, uint dictAddress, uint* nodeAddresses, uint& nodeIndex, uint8& nodeChunkIndex, uint letterNum, uint8& nodeChunkSize)
{
	char* currentAddress = dictRoot + dictAddress;
	uint bitF = *((uint*)currentAddress) & ((1 << letterNum) - 1);
	bitF = bitF - ((bitF >> 1) & 0x55555555);
	bitF = (bitF & 0x33333333) + ((bitF >> 2) & 0x33333333);
	uint numSetPartsBefore = (((bitF + (bitF >> 4)) & 0x0F0F0F0F) * 0x03030303) >> 24;
	uint* memLocation = (uint*)(currentAddress + 4 + nodeChunkSize + numSetPartsBefore);
	dictAddress += *memLocation & 0x00FFFFFF;
	nodeAddresses[++nodeIndex] = dictAddress;
	nodeChunkIndex = 0;
	nodeChunkSize = (uint8)dictRoot[dictAddress + 3] >> 4;
}

void regressWord(uint& wordIndex, char* dictRoot, uint* nodeAddresses, uint& nodeIndex, uint8& nodeChunkIndex)
{
	wordIndex--;
	assert((int)wordIndex >= 0);
	if (nodeChunkIndex == 0)
	{
		//move back one node
		nodeChunkIndex = (uint8)(dictRoot[nodeAddresses[--nodeIndex] + 3] >> 4) - 1;
		return;
	}
	nodeChunkIndex--;
}

void isWordInDict(char* word, uint wordLen, char* dictRoot, uint* nodeAddresses, uint& nodeIndex, uint& wordIndex, uint8& nodeChunkIndex)
{
	uint dictAddress = nodeAddresses[nodeIndex];
	wordIndex++;

	//check if i need to go to the next node
	uint8 nodeChunkSize = (uint8)dictRoot[dictAddress + 3] >> 4;

	if (nodeChunkIndex + 1 == nodeChunkSize)
	{
		//check that there is a next node
		uint letterNum = word[wordIndex] - 'a';
		if (!(*(uint*)(dictRoot + dictAddress) & (1 << (letterNum)))) { wordIndex--; return; }

		progressNodes(word, wordIndex, dictRoot, dictAddress, nodeAddresses, nodeIndex, nodeChunkIndex, letterNum, nodeChunkSize);
	}
	else
	{
		nodeChunkIndex++;
	}

	//get info about new node
	dictAddress = nodeAddresses[nodeIndex];
	char* nodeWordChunk = dictRoot + dictAddress + 4;

	if (word[wordIndex] != nodeWordChunk[nodeChunkIndex]) //words don't match
	{
		regressWord(wordIndex, dictRoot, nodeAddresses, nodeIndex, nodeChunkIndex);
		return;
	}
	if (nodeChunkIndex + 1 == nodeChunkSize) //node chunk at end
	{
		dictRoot[dictAddress + 3] = (uint8)dictRoot[dictAddress + 3] | 1 << 3;
		if (wordIndex + 1 == wordLen) //word at end
		{
			bool shouldContinue = (bool)((*(uint*)(dictRoot + dictAddress)) & 0x03FFFFFF);
			if (!shouldContinue) { regressWord(wordIndex, dictRoot, nodeAddresses, nodeIndex, nodeChunkIndex); }
		}
		return;
	}
}

void findWordsDownBranch(char* mem, uint memIndex, char** foundWords, uint& foundWordIndex, MemoryArena* outputMemory, char* wordToAddSoFar, uint wordToAddIndex)
{
	for (int letterNum = 0; letterNum < 26; letterNum++)
	{
		if (!(*(uint*)(mem + memIndex) & (1 << (letterNum)))) { continue; } //if the next chunk does not exist, and if it does exist, it must be one that has been found in the boggle board

		//find next chunk down
		uint numSetBitsBefore = numSetBits(*((uint*)(mem + memIndex)) & ((1 << letterNum) - 1));
		uint8 nodeWordChunkSize = (uint8)mem[memIndex + 3] >> 4;
		uint findingLocation = (memIndex + 4 + nodeWordChunkSize + (numSetBitsBefore * 3));
		uint* memLocation = (uint*)(mem + findingLocation);
		uint memIndexAfter = memIndex + (*memLocation & 0x00FFFFFF);

		if (!(((uint8)mem[memIndexAfter + 3]) & (1 << 3))) { continue; }
		mem[memIndexAfter + 3] = (uint8)mem[memIndexAfter + 3] & (~(1 << 3));
		assert((!(((uint8)mem[memIndexAfter + 3]) & (1 << 3))));

		nodeWordChunkSize = (uint8)mem[memIndexAfter + 3] >> 4;
		char* nodeWordChunk = mem + memIndexAfter + 4;

		copyString(nodeWordChunk, nodeWordChunkSize, wordToAddSoFar + wordToAddIndex);
		uint wordToAddIndexAfter = wordToAddIndex + nodeWordChunkSize;

		if ((bool)(mem[memIndexAfter + 3] & (1 << 2))) //if it's the end of a word
		{
			char* word = pushArray(outputMemory, wordToAddIndexAfter + 1, char);
			word[wordToAddIndexAfter] = 0;
			copyString(wordToAddSoFar, wordToAddIndexAfter, word);
			foundWords[foundWordIndex++] = word;
		}

		findWordsDownBranch(mem, memIndexAfter, foundWords, foundWordIndex, outputMemory, wordToAddSoFar, wordToAddIndexAfter);
	}
}

void solveThreadFunction(WorkQueue *queue, void *data)
{
	SolveThreadData* solveThreadData = (SolveThreadData*)data;
	int rootDown = solveThreadData->rootDownStart;
	int rootAcross = solveThreadData->rootAcrossStart;
	int nodesToComplete = solveThreadData->nodesToComplete;
	char* dictRoot = (char*)solveThreadData->solveThreadReadData->dictRoot;
	char* board = solveThreadData->solveThreadReadData->board;
	int width = solveThreadData->solveThreadReadData->width;
	int height = solveThreadData->solveThreadReadData->height;
	int nodesCompleted = 0;

	while (nodesCompleted++ < nodesToComplete)
	{
		char dir = 0; //direction from 0 to 7, means from right going clockwise
		uint8 checkingWordIndex = 0;
		int endDown = rootDown;
		int endAcross = rootAcross;
		char checkingWord[MAX_WORD_SIZE] = {};
		checkingWord[checkingWordIndex] = board[to1d(rootDown, rootAcross, width)];
		BoardPos path[MAX_WORD_SIZE] = {};
		path[checkingWordIndex].down = rootDown;
		path[checkingWordIndex].across = rootAcross;
		markEdgeLocations(path, rootDown, rootAcross, width, height, checkingWordIndex);

		//init dict lookup
		uint wordIndex = 0;
		uint8 nodeChunkIndex = 0;
		uint nodeAddresses[20];
		nodeAddresses[0] = 0;
		uint nodeIndex = 0;

		uint dictAddress = nodeAddresses[nodeIndex];
		uint letterNum = checkingWord[wordIndex] - 'a';
		uint8 nodeChunkSize = (uint8)dictRoot[dictAddress + 3] >> 4;
		progressNodes(checkingWord, wordIndex, dictRoot, dictAddress, nodeAddresses, nodeIndex, nodeChunkIndex, letterNum, nodeChunkSize);
		dictAddress = nodeAddresses[nodeIndex];
		dictRoot[dictAddress + 3] = (uint8)dictRoot[dictAddress + 3] | 1 << 3;

		for (;;)
		{
			assert(dir >= 0 && dir < 8);
			assert(path[checkingWordIndex].down == endDown);
			assert(path[checkingWordIndex].across == endAcross);

			if (path[checkingWordIndex].directionsBlocked[dir])
			{
				bool allDirsBlocked =
					!(!(path[checkingWordIndex].directionsBlocked[0]) |
						!(path[checkingWordIndex].directionsBlocked[1]) |
						!(path[checkingWordIndex].directionsBlocked[2]) |
						!(path[checkingWordIndex].directionsBlocked[3]) |
						!(path[checkingWordIndex].directionsBlocked[4]) |
						!(path[checkingWordIndex].directionsBlocked[5]) |
						!(path[checkingWordIndex].directionsBlocked[6]) |
						!(path[checkingWordIndex].directionsBlocked[7]));
				if (!allDirsBlocked)
				{
					do
					{
						dir = ++dir % 8;
					} while (path[checkingWordIndex].directionsBlocked[dir]);
					continue;
				}
				else
				{
					if (checkingWordIndex == 0) break; //if you have checked everything for the root, then you are done
					moveBackAndBlock(endDown, endAcross, path, checkingWordIndex, checkingWord, dir);
					assert(checkingWordIndex < wordIndex);
					regressWord(wordIndex, dictRoot, nodeAddresses, nodeIndex, nodeChunkIndex);
					continue;
				}
			}
			//advance the word
			int dirDown = 0;
			int dirAcross = 1;
			toDirs(dirDown, dirAcross, dir);
			checkingWordIndex++;
			checkingWord[checkingWordIndex] = board[to1d(endDown + dirDown, endAcross + dirAcross, width)];

			int wordSize = checkingWordIndex + 1;
			assert(checkingWordIndex <= MAX_WORD_SIZE);

			isWordInDict(checkingWord, wordSize, dictRoot, nodeAddresses, nodeIndex, wordIndex, nodeChunkIndex);

			if (wordIndex != (wordSize - 1))
			{
				checkingWord[checkingWordIndex] = 0;
				path[--checkingWordIndex].directionsBlocked[dir] = true;
				continue;
			}

			endDown = endDown + dirDown;
			endAcross = endAcross + dirAcross;
			path[checkingWordIndex].down = endDown;
			path[checkingWordIndex].across = endAcross;

			//block off all of the path it has taken if it's within reach
			for (uint i = 0; i < checkingWordIndex; i++)
			{
				int checkingPathDirDown = path[i].down - endDown;
				int checkingPathDirAcross = path[i].across - endAcross;

				if ((checkingPathDirDown == 1 || checkingPathDirDown == 0 || checkingPathDirDown == -1) &&
					(checkingPathDirAcross == 1 || checkingPathDirAcross == 0 || checkingPathDirAcross == -1))
				{
					assert(!(checkingPathDirDown == 0 && checkingPathDirAcross == 0));
					path[checkingWordIndex].directionsBlocked[dirFrom(checkingPathDirDown, checkingPathDirAcross)] = true;
				}
			}

			assert(path[checkingWordIndex].across - path[checkingWordIndex - 1].across == 1 || path[checkingWordIndex].across - path[checkingWordIndex - 1].across == 0 || path[checkingWordIndex].across - path[checkingWordIndex - 1].across == -1);
			assert(path[checkingWordIndex].down - path[checkingWordIndex - 1].down == 1 || path[checkingWordIndex].down - path[checkingWordIndex - 1].down == 0 || path[checkingWordIndex].down - path[checkingWordIndex - 1].down == -1);

			markEdgeLocations(path, endDown, endAcross, width, height, checkingWordIndex);
		}
		rootAcross++;
		if (rootAcross == width)
		{
			rootDown++;
			rootAcross = 0;
		}
	}
}

Solution* solve(char* board, int width, int height, WorkQueue* queue, MemoryArena* calcMemory, void* dictRoot, MemoryArena* outputMemory)
{
	int numWorkUnits = 1000;
	float floatStride = ((width * height) / (float)numWorkUnits);
	int stride = (((int)floatStride - floatStride) == 0) ? (int)floatStride : (int)floatStride + 1;
	SolveThreadData* solveThreadDataList = pushArray(calcMemory, numWorkUnits, SolveThreadData);
	SolveThreadReadData* solveThreadReadData = pushStruct(calcMemory, SolveThreadReadData);

	solveThreadReadData->board = board;
	solveThreadReadData->dictRoot = dictRoot;
	solveThreadReadData->width = width;
	solveThreadReadData->height = height;

	int rootDown = 0;
	int rootAcross = 0;
	int solveThreadDataListIndex = 0;

	while (to1d(rootDown, rootAcross, width) < (width * height) - 1)
	{
		SolveThreadData* solveThreadData = solveThreadDataList + solveThreadDataListIndex;

		solveThreadData->solveThreadReadData = solveThreadReadData;
		solveThreadData->rootDownStart = rootDown;
		solveThreadData->rootAcrossStart = rootAcross;
		solveThreadData->nodesToComplete = min(stride, (width * height) - (to1d(rootDown, rootAcross, width)));
		addThreadWork(queue, solveThreadFunction, (void*)solveThreadData);

		int newStart = min(to1d(rootDown, rootAcross, width) + stride, (width * height) - 1);
		rootDown = (int)(newStart / width);
		rootAcross = newStart % width;
		solveThreadDataListIndex++;
	}

	completeAllWork(queue);

	char* mem = (char*)dictRoot;
	uint memIndex = 0;
	char** foundWords = pushArray(outputMemory, 211000, char*); //allows for every word in dict
	uint foundWordIndex = 0;
	char wordToAddSoFar[MAX_WORD_SIZE] = {};
	uint wordToAddIndex = 0;

	findWordsDownBranch(mem, memIndex, foundWords, foundWordIndex, outputMemory, wordToAddSoFar, wordToAddIndex);

	uint score = 0;
	for (uint i = 0; i < foundWordIndex; i++)
	{
		uint wordSize = 0;
		while (foundWords[i][wordSize] != 0) wordSize++;
		if (wordSize == 3 || wordSize == 4)      score += 1;
		else if (wordSize == 5 || wordSize == 6)      score += wordSize - 3;
		else if (wordSize == 7)                       score += 5;
		else if (wordSize > 7)                        score += 11;
		else assert(0);
	}

	Solution* solution = pushStruct(outputMemory, Solution);
	solution->words = foundWords;
	solution->score = score;
	return solution;
}