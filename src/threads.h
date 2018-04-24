#ifndef THREADS_H
#define THREADS_H

#include "main.h"

struct WorkQueue;
typedef void workQueueCallback(WorkQueue *queue, void *data);

struct WorkQueueEntry
{
	workQueueCallback *callback;
	void *data;
};

struct WorkQueue
{
	uint volatile completionGoal;
	uint volatile completionCount;

	uint volatile nextEntryToWrite;
	uint volatile nextEntryToRead;
	void* semaphoreHandle;

	WorkQueueEntry entries[4096];
};

struct ThreadInfo
{
	int logicalThreadIndex;
	WorkQueue *queue;
};

void addThreadWork(WorkQueue *queue, workQueueCallback *callback, void *data);
void completeAllWork(WorkQueue *queue);
void makeQueue(WorkQueue &queue, ThreadInfo *threadInfo, uint threadCount);
#endif