#ifndef PLATFORM_FUNCTIONS_H
#define PLATFORM_FUNCTIONS_H

#include "main.h"
#include "utilities.h"

#define FILENAME_SIZE 260
typedef void* fileHandle;

void* allocMemory(uint size);
void freeMemory(void* memory);
bool getFile(char* filenameFilter, fileHandle& searchHandle, String outputFilename);
void closeFileHandle(fileHandle handle);
uint64 time();
uint64 timeMS();
uint64 highResTime();
uint64 highResTimeFreq();
uint getNumProcessors();
void createFolder(char* name);
char* readEntireFile(char* filename);
void freeMemory(void* memory);
void saveFile(char* name, void* mem, int size);
bool doesFileExist(char* filename);
#endif
