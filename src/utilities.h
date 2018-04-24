#ifndef UTILITIES_H
#define UTILITIES_H

#include "main.h"

//------------------------------------------------------------------------------------------
//MEMORY ARENA THINGS
//------------------------------------------------------------------------------------------
struct MemoryArena
{
	memindex size;
	uint8 *base;
	memindex used;
};

#define pushStruct(arena, type) (type *)pushMemory_(arena, sizeof(type))
#define pushArray(arena, Count, type) (type *)pushMemory_(arena, (Count)*sizeof(type))

void* pushMemory_(MemoryArena* arena, memindex size);
void initMemoryArena(MemoryArena& memArena, int size);
void freeMemoryArena(MemoryArena& memArena);
void clearMemoryArena(MemoryArena& memArena);

//------------------------------------------------------------------------------------------
//utils
//------------------------------------------------------------------------------------------
struct String
{
	char* cString;
	uint size;
	uint index;
};

char* uintToString(uint num, char* string);
char* uintToString(uint num, MemoryArena* mem);
void copyString(char* from, char* to);
void copyString(char* from, uint fromSize, char* to);
uint stringLen(char* string);
bool doWordsMatch(char* a, char* b);
char* intToString(int num, char* string);
char* intToString(int num, MemoryArena* mem);
void addString(String& string, char* toAdd);
bool isValidInt(char* string);
int stringToInt(char* string);
void clearString(char* string);
void clearString(String string);

#define min(a,b) ((a < b) ? a : b)

//------------------------------------------------------------------------------------------
//random
//------------------------------------------------------------------------------------------
struct pcgState
{
	uint64 state, increment;
};

uint rand32();
void seedRand(uint64 initState, uint64 initIncrement);
int randInt(int min, int max);

#endif
