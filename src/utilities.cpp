#include "utilities.h"
#include "platform_functions.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MEMORY ARENA THINGS
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void* pushMemory_(MemoryArena* arena, memindex size)
{
	assert((arena->used + size) <= arena->size);
	void *Result = arena->base + arena->used;
	arena->used += size;

	return Result;
}

void initMemoryArena(MemoryArena& memArena, int size)
{
	memArena = {};
	memArena.size = size;
	memArena.base = (uint8*)allocMemory((uint)memArena.size);
	assert(memArena.base);
}

void clearMemoryArena(MemoryArena& memArena)
{
	for (uint i = 0; i < memArena.used; i++)
	{
		memArena.base[i] = 0;
	}
	memArena.used = 0;
}

void freeMemoryArena(MemoryArena& memArena)
{
	freeMemory((void*)memArena.base);
	memArena = {};
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//RANDOM
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

pcgState rng;

uint rand32()
{
	uint64 oldstate = rng.state;
	rng.state = oldstate * 6364136223846793005ULL + rng.increment;
	uint xorshifted = (uint)(((oldstate >> 18u) ^ oldstate) >> 27u);
	uint rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((rot | (1 << 31)) & 31));
}

void seedRand(uint64 initState, uint64 initIncrement)
{
	rng.state = 0U;
	rng.increment = (initIncrement << 1u) | 1u;
	rand32();
	rng.state += initState;
	rand32();
}

int randInt(int minNum, int maxNum)
{
	assert(minNum <= maxNum);
	int result = minNum + (rand32() % ((maxNum + 1) - minNum));
	return result;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MATHS FUNCTIONS
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint64 intPow(int num, int power)
{
	if (power == 0)
	{
		return 1;
	}
	uint64 result = num;
	for (int i = 1; i < power; i++)
	{
		result *= num;
	}
	return result;
}

int roundToI(float x)
{
	int result;
	if (x < 0)
	{
		result = (int)(x - 0.5);
	}
	else
	{
		result = (int)(x + 0.5);
	}
	return result;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//STRING HANDLING
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool isValidInt(char* string)
{
	if (!string) return false;

	char* at = string;
	for (; *at; at++)
	{
		if (!((*at >= '0') && (*at <= '9')))
		{
			return false;
		}
	}
	return true;
}

int stringToInt(char* string)
{
	for (; *string == ' '; string++);

	bool isNegative = false;
	if (*string == '-')
	{
		isNegative = true;
		string++;
	}

	int result = 0;
	for (; *string; string++)
	{
		int digit = 0;
		if (*string >= '0' && *string <= '9')
		{
			digit = *string - '0';
		}
		else break;

		result *= 10;
		result += digit;
	}

	if (isNegative)
	{
		result *= -1;
	}

	return (result);
}

uint stringLen(char* string)
{
	int result = 0;
	while (*string) { result++; string++; }
	return result;
}

void copyString(char* from, char* to)
{
	char* at = from;
	int i = 0;
	while (*at)
	{
		to[i] = at[0];
		at++;
		i++;
	}
	to[i] = 0;
}

void addString(String& string, char* toAdd)
{
	uint toAddIndex = 0;
	while (toAdd[toAddIndex])
	{
		string.cString[string.index++] = toAdd[toAddIndex++];
		assert(string.index <= string.size);
	}
}

void flipString(char* string)
{
	int stringSize = stringLen(string);
	char string2[1024] = {};
	int string2Index = 0;
	for (int i = stringSize - 1; i >= 0; i--)
	{
		string2[string2Index++] = string[i];
	}
	copyString(string2, string);
}

void clearString(char* string)
{
	uint i = 0;
	while (string[i])
	{
		string[i++] = 0;
	}
}

int digitsInInt(int num)
{
	int minus = 0;
	if (num < 0)
	{
		minus = 1;
	}
	for (int i = 0;; i++)
	{
		int takeNum = (num % intPow(10, i + 1));
		num -= takeNum;
		if (num == 0)
		{
			return i + 1 + minus;
		}
	}
}

char* uintToString(uint num, char* string)
{
	clearString(string);

	for (uint i = 0;; i++)
	{
		uint takeNum = (num % intPow(10, i + 1));
		string[i] = '0' + (char)(takeNum / intPow(10, i));
		num -= takeNum;
		if (num == 0)
		{
			break;
		}
	}

	flipString(string);
	return string;
}

char* intToString(int num, char* string)
{
	int minusSignRoom = 0;
	if (num < 0)
	{
		string[0] = '-';
		num *= -1;
		minusSignRoom = 1;
	}

	string += minusSignRoom;
	uintToString(num, string);
	string -= minusSignRoom;

	return string;
}

char* uintToString(uint num, MemoryArena* mem)
{
	uint intSize = digitsInInt(num);
	char* result = pushArray(mem, intSize + 1, char);
	return uintToString(num, result);
}

char* intToString(int num, MemoryArena* mem)
{
	uint intSize = digitsInInt(num);
	char* result = pushArray(mem, intSize + 1, char);
	return intToString(num, result);
}

bool doWordsMatch(char* a, char* b)
{
	for (uint index = 0;; index++)
	{
		if (a[index] != b[index])
		{
			return false;
		}
		else if (a[index] == 0)
		{
			assert(index);
			return true;
		}
	}
}

void copyString(char* from, uint fromSize, char* to)
{
	char* at = from;
	uint i = 0;
	while (i < fromSize)
	{
		to[i] = at[0];
		at++;
		i++;
	}
}

void clearString(String string)
{
	clearString(string.cString);
	string.index = 0;
}