#include "platform_functions.h"
#include <windows.h>

void* allocMemory(uint size)
{
	void* result = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	return result;
}

void freeMemory(void* memory)
{
	if (memory)
	{
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}

bool doesFileExist(char* filename)
{
	WIN32_FIND_DATA findData;
	HANDLE searchHandle = FindFirstFile(filename, &findData);
	if (searchHandle == INVALID_HANDLE_VALUE) { return false; }
	return true;
}

char* readEntireFile(char* filename)
{
	char* memory = 0;

	HANDLE fileHandle;
	fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			memory = (char*)allocMemory((uint)(fileSize.QuadPart + 1)); //should maybe use HeapAlloc
			if (memory)
			{
				DWORD bytesRead = 0;
				if (!ReadFile(fileHandle, memory, (DWORD)fileSize.QuadPart, &bytesRead, 0))
				{
					freeMemory(memory);
					memory = 0;
				}
			}
		}
		memory[fileSize.QuadPart] = 0;
		CloseHandle(fileHandle);
	}
	return memory;
}

void saveFile(char* name, void* mem, int size)
{
	assert(mem);
	HANDLE fileHandle = CreateFileA(name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	DWORD bytesWritten = 0;
	int done = WriteFile(fileHandle, mem, size, &bytesWritten, 0);
	assert(done);
	CloseHandle(fileHandle);
}

uint64 time()
{
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);

	FILETIME FileTime;
	SystemTimeToFileTime(&SystemTime, &FileTime);

	uint64 result = {};
	result = result | ((uint64)FileTime.dwHighDateTime << 32);
	result = result | FileTime.dwLowDateTime;

	return result;
}

uint64 timeMS()
{
	uint64 result = time() / 10000;
	return result;
}

uint64 highResTime()
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.QuadPart;
}

uint64 highResTimeFreq()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	int64 frequency = freq.QuadPart;
	return frequency;
}

uint getNumProcessors()
{
	SYSTEM_INFO systemInfo = {};
	GetSystemInfo(&systemInfo);
	return systemInfo.dwNumberOfProcessors;
}

void createFolder(char* name)
{
	CreateDirectory(name, 0);
}

bool getFile(char* filenameFilter, fileHandle& searchHandle, String outputFilename)
{
	WIN32_FIND_DATA findData = {};
	if (!searchHandle)
	{
		searchHandle = (fileHandle)FindFirstFile(filenameFilter, &findData);
		if (searchHandle == INVALID_HANDLE_VALUE) { return false; }
	}
	else
	{
		if (!FindNextFile(searchHandle, &findData))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{
				return false; 
			}
		}
	}

	clearString(outputFilename);
	addString(outputFilename, findData.cFileName);
	return true;
}

void closeFileHandle(fileHandle handle)
{
	if (handle && handle != INVALID_HANDLE_VALUE)
	{
		FindClose((HANDLE)handle);
	}
}
