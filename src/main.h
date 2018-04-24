#ifndef MAIN_H
#define MAIN_H

#define MAX_WORD_SIZE 24 //hard coded from the dict
#define KILOBYTES(x) x * 1024
#define	MEGABYTES(x) KILOBYTES(x) * 1024

#define DEBUG 0
#define TIMING 0

#if TIMING
#define BEGIN_TIMED_BLOCK(NAME) uint64 StartCycleCount##NAME = __rdtsc();
#define END_TIMED_BLOCK_(NAME, ID) debugTimers[ID].name = #NAME; debugTimers[ID].totalTime += ((__rdtsc()) - (StartCycleCount##NAME)); debugTimers[ID].hits++;
#define END_TIMED_BLOCK(NAME) END_TIMED_BLOCK_(NAME,  __COUNTER__)

#define BEGIN_SINGLE_THREAD_TIMED_BLOCK(NAME) uint64 StartCycleCount##NAME = 0; if (!THREADING) {StartCycleCount##NAME = __rdtsc();};
#define END_SINGLE_THREAD_TIMED_BLOCK(NAME) if (!THREADING) { END_TIMED_BLOCK(NAME); }

#define BEGIN_MULTI_THREAD_TIMED_BLOCK(NAME) uint64 StartCycleCount##NAME = 0; if (THREADING) { StartCycleCount##NAME = __rdtsc(); };
#define END_MULTI_THREAD_TIMED_BLOCK(NAME) if (THREADING) { END_TIMED_BLOCK(NAME); }
#else
#define BEGIN_TIMED_BLOCK(x)
#define END_TIMED_BLOCK(x)
#define BEGIN_SINGLE_THREAD_TIMED_BLOCK(x)
#define END_SINGLE_THREAD_TIMED_BLOCK(x)
#define BEGIN_MULTI_THREAD_TIMED_BLOCK(NAME)
#define END_MULTI_THREAD_TIMED_BLOCK(NAME)
#endif

#if DEBUG
#define assert(x) if (!(x)) {*(int*)1 = 0;}
#else
#define assert(x)
#endif

typedef size_t memindex;
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef unsigned int uint;
typedef long long int64;
typedef unsigned long long uint64;

#define global_var static
#define arraySize(x) ((int)(sizeof(x) / sizeof(x[0])))

#if TIMING
struct DebugTimer
{
	char* name;
	uint64  hits = 0;
	uint64 totalTime = 0;
};

global_var DebugTimer debugTimers[64];
#endif
#endif