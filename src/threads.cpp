#include <windows.h>
#include <intrin.h>

#include "main.h"
#include "threads.h"

DWORD WINAPI threadFunction(LPVOID lpParameter);

void makeQueue(WorkQueue &queue, ThreadInfo *threadInfo, uint threadCount)
{
	uint initialCount = 0;
	queue.semaphoreHandle = CreateSemaphoreEx(0, initialCount, threadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
	for (uint threadIndex = 0; threadIndex < threadCount; ++threadIndex)
	{
		ThreadInfo *info = threadInfo + threadIndex;
		info->queue = &queue;
		info->logicalThreadIndex = threadIndex;

		DWORD threadID;
		HANDLE threadHandle = CreateThread(0, 0, threadFunction, info, 0, &threadID);
		CloseHandle(threadHandle);
	}
}

void addThreadWork(WorkQueue *queue, workQueueCallback *callback, void *data)
{
	uint newNextEntryToWrite = (queue->nextEntryToWrite + 1) % arraySize(queue->entries);
	assert(newNextEntryToWrite != queue->nextEntryToRead);
	WorkQueueEntry *entry = queue->entries + queue->nextEntryToWrite;
	entry->callback = callback;
	entry->data = data;
	queue->completionGoal++;
	_WriteBarrier();
	queue->nextEntryToWrite = newNextEntryToWrite;
	ReleaseSemaphore(queue->semaphoreHandle, 1, 0); //costs around 5000 cycles
}

bool doNextWork(WorkQueue *queue)
{
	bool shouldSleep = false;
	uint originalNextEntryToRead = queue->nextEntryToRead;
	if (originalNextEntryToRead != queue->nextEntryToWrite)
	{
		uint newNextEntryToRead = (originalNextEntryToRead + 1) % arraySize(queue->entries);
		uint index = InterlockedCompareExchange((LONG volatile *)&queue->nextEntryToRead, newNextEntryToRead, originalNextEntryToRead);
		if (index == originalNextEntryToRead)
		{
			WorkQueueEntry entry = queue->entries[index];
			entry.callback(queue, entry.data);
			InterlockedIncrement((LONG volatile *)&queue->completionCount);
		}
	}
	else
	{
		shouldSleep = true;
	}

	return shouldSleep;
}

void completeAllWork(WorkQueue *queue)
{
	while (queue->completionGoal != queue->completionCount)
	{
		doNextWork(queue);
	}

	queue->completionGoal = 0;
	queue->completionCount = 0;
}

DWORD WINAPI threadFunction(LPVOID lpParameter)
{
	ThreadInfo *threadInfo = (ThreadInfo*)lpParameter;

	for (;;)
	{
		if (doNextWork(threadInfo->queue))
		{
			WaitForSingleObjectEx(threadInfo->queue->semaphoreHandle, INFINITE, FALSE);
		}
	}
}
