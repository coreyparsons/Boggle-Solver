#include <stdio.h> //printf and gets
#include <stdlib.h> //malloc and free

#include "main.h"
#include "platform_functions.h"
#include "threads.h"
#include "boggle.h"
#include "utilities.h"

struct Command
{
	char* name;
	char* mainDescription;
	char** argNames;
	char** argDescriptions;
	bool(*function)(uint, char**, char*&, void*);
	uint numArgs;
};

void printRowDivider(uint width)
{
	printf("|");
	for (uint i = 0; i < width - 2; ++i)
	{
		printf("-");
	}
	printf("|\n");
}

void printSpaces(uint num, char* endString)
{
	for (uint spaceIndex = 0; spaceIndex < num; ++spaceIndex)
	{
		printf(" ");
	}
	printf("%s", endString);
}

void printSpaces(uint num)
{	
	printSpaces(num, "");
}

void printRowSection(char* string, uint rowSpace)
{
	printf("| ");
	printf("%s", string);
	printSpaces(rowSpace - stringLen(string));
}

void printRow(char** strings, uint numStrings, uint rowSpace)
{
	for (uint i = 0; i < numStrings; ++i)
	{
		printRowSection(strings[i], rowSpace);
	}
	printf("|\n");
}

bool commandHelp(uint numArgs, char** args, char*& error, void* commandData)
{
	Command* commandList = (Command*)commandData;
	if (numArgs == 1)
	{
		char* commandName = args[0];
		if (!commandName)
		{
			error = "Invalid command name";
			return false;
		}

		Command* command = commandList;
		bool isError = true;
		for (; command->name; ++command)
		{
			if (doWordsMatch(command->name, commandName))
			{
				isError = false;
				break;
			}
		}

		if (isError)
		{
			error = "Invalid command name";
			return false;
		}

		printf("%s\n", command->mainDescription);

		if (command->numArgs)
		{
			printf("\narguments:\n");
			for (uint i = 0; i < command->numArgs; ++i)
			{
				printf("%s - ", command->argNames[i]);
				printf("%s\n", command->argDescriptions[i]);
			}
		}
		printf("\n");
	}
	else
	{
		printf("This program is centred around generating and solving boggle boards quickly\n");
		printf("Use any of these commands by typing the name, followed by the extra arguments the command needs (seperated with spaces)\n");
		printf("If you want to know about a specific command use help [command name] for information\n\n");

		Command* command = commandList;
		uint rowSpace = 31;
		uint spacesNeeded = 0;
		uint rowDividerSize = ((rowSpace + 2) * 3) + 1;
		char* titles[] = { "Name", "Arguments", "Description" };

		printRowDivider(rowDividerSize);
		printRow((char**)titles, arraySize(titles), rowSpace);
		printRowDivider(rowDividerSize);

		for (; command->name; ++command)
		{
			//command name
			printRowSection(command->name, rowSpace);
			printf("| ");

			//arguments
			uint amountAdded = 0;
			for (uint i = 0; i < command->numArgs; ++i)
			{
				char* addString = "";
				if (i < (command->numArgs - 1))
				{
					addString = ", ";
				}
				printf("%s%s", command->argNames[i], addString);
				amountAdded += stringLen(command->argNames[i]) + stringLen(addString);
			}
			printSpaces(rowSpace - amountAdded, "| ");

			//description
			uint descriptionSize = stringLen(command->mainDescription);
			uint extraSpace = 0;
			for (uint i = 0; i < descriptionSize; ++i)
			{
				if (i % (rowSpace - 1) == 0 && command->mainDescription[i] == ' ')
				{
					++i;
					++extraSpace;
				}
				printf("%c", command->mainDescription[i]);
				if (i % (rowSpace - 1) == (rowSpace - 2))
				{
					printf(" |\n| ");
					for (uint j = 0; j < 2; ++j)
					{
						printSpaces(rowSpace, "| ");
					}
				}
				if (i == (descriptionSize - 1))
				{
					spacesNeeded = (rowSpace - (i % (rowSpace - 1))) - 2 + extraSpace;
					printSpaces(spacesNeeded, " |\n");
				}
			}
			printRowDivider(rowDividerSize);
		}

		printf("\nAny argument that is a number can be replaced with r(min,max) for a random number between those bounds\n");
		printf("For Example you can generate a board of random height by typing \"gen 10 r(5,10)\"\n\n");
	}
	return false;
}

bool commandExit(uint numArgs, char** args, char*& error, void* commandData)
{
	return true;
}

void getFormattedBoardDims(char* formattedBoard, uint& width, uint& height)
{
	width = 0;
	height = 0;
	bool isValidBoard = true;
	uint charCount = 0;
	{
		uint rawBoardIndex = 0;
		while (formattedBoard[rawBoardIndex] != 0)
		{
			char currentChar = formattedBoard[rawBoardIndex];
			if (currentChar == '\r' || currentChar == '\n')
			{
				if (width == 0)
				{
					width = rawBoardIndex;
				}
			}
			else if (currentChar >= 'a' && currentChar <= 'z')
			{
				++charCount;
			}
			else
			{
				isValidBoard = false;
				break;
			}
			++rawBoardIndex;
		}
		if (width == 0)
		{
			isValidBoard = false;
		}
		else if ((charCount / width) != (int)(charCount / width))
		{
			isValidBoard = false;
		}
		if (!isValidBoard)
		{
			width = 0;
			height = 0;
			return;
		}
	}

	height = (int)(charCount / width);
}

void unformatBoard(char* outputBoard, char* formattedBoard)
{
	uint formattedBoardIndex = 0;
	uint boardIndex = 0;
	while (formattedBoard[formattedBoardIndex] != 0)
	{
		char letter = formattedBoard[formattedBoardIndex];
		if (letter != '\r' && letter != '\n')
		{
			outputBoard[boardIndex++] = letter;
		}
		++formattedBoardIndex;
	}
}

//NOTE: the returned value needs to be freed
char* unformatBoard(char* formattedBoard, uint& width, uint& height)
{
	getFormattedBoardDims(formattedBoard, width, height);
	int boardSize = width * height;

	if (!boardSize)
	{
		return 0;
	}

	char* board = (char*)malloc(boardSize);
	unformatBoard(board, formattedBoard);

	return board;
}

void solveFormattedBoardToFile(char* rawBoard, char* filename, WorkQueue* queue, void* dictRoot, MemoryArena& calcMemory, MemoryArena& outputMemory, char*& error)
{
	uint width = 0;
	uint height = 0;
	char* board = unformatBoard(rawBoard, width, height);
	if (!board)
	{
		error = "Invalid Board";
		return;
	}

	Solution* solution = pushStruct(&outputMemory, Solution);

	uint64 beginTime = highResTime();

	solution = solve(board, width, height, queue, &calcMemory, dictRoot, &outputMemory);

	uint64 endTime = highResTime();
	int64 timeElapsed = endTime - beginTime;
	int64 freq = highResTimeFreq();
	int64 msElapsed = (1000 * timeElapsed / freq);

#if TIMING //debug timing info
	double totalMegacyclesCovered = 0;
	for (int i = 0; i < arraySize(debugTimers); i++)
	{
		if (debugTimers[i].name != 0)
		{
			double averageCycles = ((double)debugTimers[i].totalTime / (double)debugTimers[i].hits);
			double totalMC = (debugTimers[i].totalTime / 1000000.0);

			totalMegacyclesCovered += totalMC;

			printf("\n==================================\n");
			printf("%s\n", (char*)(debugTimers[i].name));
			if (debugTimers[i].hits > 1)
			{
				printf("Hits: %llu\n", (int64)(debugTimers[i].hits));
				printf("Total Megacycles: %f\n", totalMC);
				printf("Average Cycles: %f\n", averageCycles);
			}
			else
			{
				printf("Megacycles: %f\n", totalMC);
			}
			printf("==================================\n");
		}
	}

	printf("\n\n");
	printf("Covered: %f megacycles, out of a possible %llu\n", totalMegacyclesCovered, totalMegacycles);
#endif

	//add found words to output file
	uint numFoundWords = 0;
	char** foundWords = solution->words;
	while (foundWords[numFoundWords])
	{
		++numFoundWords;
	}

	int largestWordNum = 0;
	int largestWordSize = 0;
	uint i = 0;
	while (foundWords[i])
	{
		int sizeIndex = 0;

		while (foundWords[i][sizeIndex] != 0)
		{
			sizeIndex++;
		}

		if (sizeIndex > largestWordSize)
		{
			largestWordSize = sizeIndex;
			largestWordNum = i;
		}
		i++;
	}

	String output = {};
	output.size = (numFoundWords * 12) + 1024; //assuming average word size is 12, adding 1024 chars for the start bit
	output.cString = pushArray(&outputMemory, output.size, char);

	addString(output, "==================================\r\n");
	addString(output, "Board size:   ");

	addString(output, intToString((int)width, &outputMemory));
	addString(output, " X ");
	addString(output, intToString((int)height, &outputMemory));

	addString(output, "\r\nMilliseconds: ");
	addString(output, intToString((int)msElapsed, &outputMemory));

	addString(output, "\r\nFound words:  ");
	addString(output, intToString(numFoundWords, &outputMemory));

	if (numFoundWords)
	{
		addString(output, "\r\nLargest word: ");
		addString(output, foundWords[largestWordNum]);
	}

	addString(output, "\r\nScore:        ");
	addString(output, intToString(solution->score, &outputMemory));

	addString(output, "\r\n==================================\r\n");
	addString(output, "\r\n");

	uint wordListIndex = 0;
	while (foundWords[wordListIndex])
	{
		char* word = foundWords[wordListIndex];
		uint wordIndex = 0;
		addString(output, word);
		addString(output, " ");
		++wordListIndex;
	}

	String filenameOutput = {};
	char filenameOutputString[FILENAME_SIZE] = {};
	filenameOutput.cString = filenameOutputString;
	filenameOutput.size = arraySize(filenameOutputString);

	addString(filenameOutput, "Solved/");
	addString(filenameOutput, filename);
	createFolder("Solved");
	saveFile(filenameOutput.cString, output.cString, output.index);
	printf("%s\n", filenameOutput.cString);

	//this doesn't seem like a good solution
	free(board);
	clearMemoryArena(calcMemory);
	clearMemoryArena(outputMemory);
	freeMemory(rawBoard);
}

bool commandSolve(uint numArgs, char** args, char*& error, void* commandData)
{
	WorkQueue* queue = (WorkQueue*)commandData;

	void* dictRoot = readEntireFile("dictionary.bin");
	if (!dictRoot)
	{
		error = "Couldn't find \"dictionary.bin\"";
		return false;
	}

	MemoryArena calcMemory;
	initMemoryArena(calcMemory, KILOBYTES(50));

	MemoryArena outputMemory;
	initMemoryArena(outputMemory, MEGABYTES(3));

	bool searchingAllFiles = !(numArgs == 1);

	if (searchingAllFiles)
	{
		String outputFilename = {};
		char outputFilenameString[FILENAME_SIZE] = {};
		outputFilename.size = arraySize(outputFilenameString);
		outputFilename.cString = outputFilenameString;
		
		char* folders[] = { "", "Generated/" };
		char* filenameEnd = "*.txt";

		for (uint fileIndex = 0; fileIndex < arraySize(folders); ++fileIndex)
		{
			String filename = {};
			char filenameString[20] = {};
			filename.size = arraySize(filenameString);
			filename.cString = filenameString;

			addString(filename, folders[fileIndex]);
			addString(filename, filenameEnd);

			fileHandle searchHandle = {};
			while (getFile(filename.cString, searchHandle, outputFilename))
			{
				char* rawBoard = 0;
				String buffer = {};
				char bufferString[50] = {};
				buffer.cString = bufferString;
				buffer.size = arraySize(bufferString);

				addString(buffer, folders[fileIndex]);
				addString(buffer, outputFilename.cString);
				rawBoard = (char*)readEntireFile(buffer.cString);
				if (!rawBoard) continue;

				char* solveError = 0;
				solveFormattedBoardToFile(rawBoard, outputFilename.cString, queue, dictRoot, calcMemory, outputMemory, solveError);
				if (solveError) continue;
			}
			closeFileHandle(searchHandle);
		}
	}
	else
	{
		String filename = {};
		char filenameString[64] = {};
		filename.cString = filenameString;
		filename.size = arraySize(filenameString);
		
		uint argLen = stringLen(args[0]);
		char* argEnd = args[0] + argLen - 4;
		if (doWordsMatch(argEnd, ".txt"))
		{
			argEnd[0] = 0;
		}

		addString(filename, args[0]);
		addString(filename, ".txt");

		char* rawBoard = (char*)readEntireFile(filename.cString);

		if (!rawBoard)
		{
			String folderFilename = {};
			char folderFilenameString[64] = {};
			folderFilename.cString = folderFilenameString;
			folderFilename.size = arraySize(folderFilenameString);

			addString(folderFilename, "Generated/");
			addString(folderFilename, args[0]);
			addString(folderFilename, ".txt");
			rawBoard = (char*)readEntireFile(folderFilename.cString);
			if (!rawBoard)
			{
				error = "Couldn't find board"; //board is empty or file doesn't exit
				return false;
			}
		}

		solveFormattedBoardToFile(rawBoard, filename.cString, queue, dictRoot, calcMemory, outputMemory, error);
		if (error) return false;
	}

	freeMemory(dictRoot);
	return false;
}

uint getStringUntilChar(char* inputString, char* outputString, char stoppingChar, bool& error)
{
	error = false;
	uint result = 0;
	for (; *inputString != stoppingChar; ++inputString)
	{
		if (*inputString == 0)
		{
			error = true;
			return 0;
		}
		*outputString++ = *inputString;
		++result;
	}
	return result;
}

char* fillIntArg(char* arg, uint& output, uint& minOutput, uint& maxOutput)
{
	char* errorCode = 0;
	if (isValidInt(arg))
	{
		output = stringToInt(arg);
	}
	else
	{
		if (!arg || !(arg[0] == 'r' && arg[1] == '('))
		{
			return "Invalid argument input";
		}

		bool isError = false;

		char min[10] = {};
		uint toAdd = getStringUntilChar(arg + 2, min, ',', isError);
		if (toAdd == 0 || isError || !isValidInt(min)) return "Invalid random";
		minOutput = stringToInt(min);

		char max[10] = {};
		toAdd = getStringUntilChar(arg + 3 + toAdd, max, ')', isError);
		if (toAdd == 0 || isError || !isValidInt(max)) return "Invalid random";
		maxOutput = stringToInt(max);

		if (minOutput > maxOutput)
		{
			errorCode = "Invalid random (min must be lower than max)";
		}
	}
	return errorCode;
}

bool commandGen(uint numArgs, char** args, char*& error, void* commandData)
{
	if (numArgs < 2)
	{
		error = "Not enough arguments";
		return false;
	}

	pcgState* rng = (pcgState*)commandData;

	uint width = 0;
	uint widthMin = 0;
	uint widthMax = 0;
	error = fillIntArg(args[0], width, widthMin, widthMax);
	if (error) return false;

	uint height = 0;
	uint heightMin = 0; 
	uint heightMax = 0;
	error = fillIntArg(args[1], height, heightMin, heightMax);
	if (error) return false;

	uint numBoards = 1;
	if (numArgs == 3)
	{
		uint numBoardsMin = 0;
		uint numBoardsMax = 0;
		error = fillIntArg(args[2], numBoards, numBoardsMin, numBoardsMax);
		if (error) return false;
		if (numBoardsMin)
		{
			numBoards = randInt(numBoardsMin, numBoardsMax);
		}
	}

	for (uint boardsIndex = 0; boardsIndex < numBoards; ++boardsIndex)
	{
		if (widthMin)
		{
			width = randInt(widthMin, widthMax);
		}
		if (heightMin)
		{
			height = randInt(heightMin, heightMax);
		}

		uint boardSize = width * height;
		char* board = (char*)malloc(boardSize + (2 * height));

		uint boardIndex = 0;
		for (uint i = 0; i < boardSize; i++)
		{
			board[boardIndex++] = 'a' + (rand32() % 26);
			if (i % width == (width - 1))
			{
				board[boardIndex++] = '\r';
				board[boardIndex++] = '\n';
			}
		}

		char intStr[10] = {};

		String outBuffer = {};
		char outBufferString[50] = {};
		outBuffer.cString = outBufferString;
		outBuffer.size = arraySize(outBufferString);

		createFolder("Generated");
		addString(outBuffer, "Generated/");
		addString(outBuffer, uintToString(width, intStr));
		addString(outBuffer, "X");
		addString(outBuffer, uintToString(height, intStr));

		//add unique identifying number
		for (uint identifier = 0;; ++identifier)
		{
			String checkingBuffer = {};
			char checkingBufferString[50] = {};
			checkingBuffer.cString = checkingBufferString;
			checkingBuffer.size = arraySize(checkingBufferString);

			addString(checkingBuffer, outBuffer.cString);
			if (identifier)
			{
				addString(checkingBuffer, " ");
				addString(checkingBuffer, uintToString(identifier, intStr));
			}
			addString(checkingBuffer, ".txt");

			if (!doesFileExist(checkingBuffer.cString))
			{
				saveFile(checkingBuffer.cString, board, boardIndex);
				printf("%s\n", checkingBuffer.cString);
				break;
			}
		}
		free(board);
	}
	return false;
}

//NOTE: the pointer returned will need to be freed
char* formatRawBoard(char* rawBoard, uint width, uint height, uint& formattedBoardSize)
{
	uint boardSize = width * height;
	formattedBoardSize = boardSize + (2 * height);
	char* formattedBoard = (char*)malloc(formattedBoardSize);

	uint formattedBoardIndex = 0;
	uint boardIndex = 0;
	for (uint i = 0; i < boardSize; i++)
	{
		formattedBoard[formattedBoardIndex++] = rawBoard[boardIndex++];
		if (i % width == (width - 1))
		{
			formattedBoard[formattedBoardIndex++] = '\r';
			formattedBoard[formattedBoardIndex++] = '\n';
		}
	}

	return formattedBoard;
}

char* getValOrRandom(char* input, uint& value)
{
	value = 0;
	uint valueMin = 0;
	uint valueMax = 0;
	char* error = fillIntArg(input, value, valueMin, valueMax);
	if (error) return error;

	if (value == 0)
	{
		value = randInt(valueMin, valueMax);
	}

	return 0;
}

bool commandFindBest(uint numArgs, char** args, char*& error, void* commandData)
{
	uint64 startTime = timeMS();

	WorkQueue* queue = (WorkQueue*)commandData;

	if (numArgs < 2)
	{
		error = "Not enough arguments";
		return false;
	}

	uint width = 0;
	error = getValOrRandom(args[0], width);
	if (error) return false;

	uint height = 0;
	error = getValOrRandom(args[1], height);
	if (error) return false;

	uint completionSeconds = 0;

	if (numArgs == 3)
	{
		error = getValOrRandom(args[2], completionSeconds);
		if (error) return false;
	}

	uint boardSize = width*height;

	MemoryArena boardMemory;
	initMemoryArena(boardMemory, boardSize);

	MemoryArena calcMemory;
	initMemoryArena(calcMemory, KILOBYTES(50));

	MemoryArena outputMemory;
	initMemoryArena(outputMemory, MEGABYTES(3));

	void* dictRoot = readEntireFile("dictionary.bin");
	if (!dictRoot)
	{
		error = "Couldn't find \"dictionary.bin\"";
		return false;
	}

	MemoryArena bestBoardMemory = {};
	initMemoryArena(bestBoardMemory, boardSize);

	String filename = {};
	char filenameString[FILENAME_SIZE] = {};
	filename.cString = filenameString;
	filename.size = arraySize(filenameString);

	createFolder("Generated");
	addString(filename, "Generated/best");
	addString(filename, intToString(width, &calcMemory));
	addString(filename, "x");
	addString(filename, intToString(height, &calcMemory));
	addString(filename, ".txt");

	char* formattedBoard = (char*)readEntireFile(filename.cString);

	char* bestBoard = 0;
	uint bestScore = 0;
	if (formattedBoard)
	{
		bestBoard = pushArray(&bestBoardMemory, boardSize, char);
		unformatBoard(bestBoard, formattedBoard);

		Solution* solution = solve(bestBoard, width, height, queue, &calcMemory, dictRoot, &outputMemory);
		bestScore = solution->score;
	}

	clearMemoryArena(calcMemory);
	clearMemoryArena(outputMemory);

	char* board = 0;
	uint boardsChecked = 0;

	while (((timeMS() - startTime) < (completionSeconds * 1000)) || (completionSeconds == 0))
	{
		board = pushArray(&boardMemory, boardSize, char);
		for (uint boardIndex = 0; boardIndex < boardSize; ++boardIndex)
		{
			board[boardIndex] = randInt((int)'a', (int)'z');
		}

		Solution* solution = solve(board, width, height, queue, &calcMemory, dictRoot, &outputMemory);

		if (solution->score > bestScore)
		{
			//swap board memory and best board memory
			MemoryArena intermediateMemory = {};
			intermediateMemory = bestBoardMemory;
			bestBoardMemory = boardMemory;
			boardMemory = intermediateMemory;

			bestBoard = board;
			bestScore = solution->score;

			if (completionSeconds == 0)
			{
				uint formattedBoardSize = 0;
				char* formattedBoardToSave = formatRawBoard(bestBoard, width, height, formattedBoardSize);
				saveFile(filename.cString, (void*)formattedBoardToSave, formattedBoardSize);
			}
		}

		clearMemoryArena(boardMemory);
		clearMemoryArena(calcMemory);
		clearMemoryArena(outputMemory);

		++boardsChecked;
	}

	uint formattedBoardSize = 0;
	char* formattedBoardToSave = formatRawBoard(bestBoard, width, height, formattedBoardSize);
	saveFile(filename.cString, (void*)formattedBoardToSave, formattedBoardSize);

	free(formattedBoardToSave);

	printf("Boards Checked: %u\n", boardsChecked);
	printf("Board Score:    %u\n", bestScore);

	freeMemoryArena(boardMemory);
	freeMemoryArena(bestBoardMemory);
	freeMemoryArena(calcMemory);
	freeMemoryArena(outputMemory);
	return false;
}

//NOTE: this is not memory safe, should just be used for initialising console, as the console will last for the duration of the program anyway
char** makeCharList(char** stringList, uint stringListCount)
{
	char** result = (char**)malloc(stringListCount * sizeof(char**));
	for (uint i = 0; i < stringListCount; ++i)
	{
		uint stringLength = stringLen(stringList[i]);
		char* string = (char*)malloc(stringLength + 1);
		for (uint j = 0; j < stringLength; ++j)
		{
			string[j] = stringList[i][j];
		}
		string[stringLength] = 0;
		result[i] = string;
	}
	return result;
}

int main()
{
	//init threads
	uint threadCount = getNumProcessors() - 1;
	ThreadInfo* threadInfo = (ThreadInfo*)malloc(threadCount * sizeof(ThreadInfo));
	WorkQueue queue = {};
	makeQueue(queue, threadInfo, threadCount);

	seedRand(time(), ((((uint64)&queue) << 32) | ((uint64)&seedRand)));

	Command commands[20] = {};
	Command* currentCommand = commands;

	{
		currentCommand->name = "help";
		currentCommand->mainDescription = "Gives general info about what you can do in this program, or specific info for a command.";
		currentCommand->numArgs = 1;
		char* stackArgNames[] = { "command" };
		char* stackArgDescriptions[] = { "The command that you would like specific help with" };
		currentCommand->function = commandHelp;

		currentCommand->argNames = makeCharList((char**)stackArgNames, currentCommand->numArgs);
		currentCommand->argDescriptions = makeCharList((char**)stackArgDescriptions, currentCommand->numArgs);
		++currentCommand;
	}

	{
		currentCommand->name = "solve";
		currentCommand->mainDescription = "Solves a specific boggle board or all boards.";
		currentCommand->numArgs = 1;
		char* stackArgNames[] = { "filename" };
		char* stackArgDescriptions[] = { "The filename of the board you want to solve. Leave blank to solve all boards." };
		currentCommand->function = commandSolve;

		currentCommand->argNames = makeCharList((char**)stackArgNames, currentCommand->numArgs);
		currentCommand->argDescriptions = makeCharList((char**)stackArgDescriptions, currentCommand->numArgs);
		++currentCommand;
	}

	{
		currentCommand->name = "gen";
		currentCommand->mainDescription = "Generates a board of specified size, with random letters.";
		currentCommand->numArgs = 3;
		char* stackArgNames[] = { "width", "height", "num" };
		char* stackArgDescriptions[] = { "The width of the generated board", "The height of the generated board", "The number of boards to generate. Leave blank to generate 1 board" };
		currentCommand->function = commandGen;

		currentCommand->argNames = makeCharList((char**)stackArgNames, currentCommand->numArgs);
		currentCommand->argDescriptions = makeCharList((char**)stackArgDescriptions, currentCommand->numArgs);
		++currentCommand;
	}

	{
		currentCommand->name = "find_best";
		currentCommand->mainDescription = "Keeps trying to find the board with the highest score.";
		currentCommand->numArgs = 3;
		char* stackArgNames[] = { "width", "height", "time" };
		char* stackArgDescriptions[] = { "The width of the board", "The height of the board", "The seconds that the program should spend finding the best board. Can be left blank.\n       NOTE: the program only checks the time after completing a board. So bigger boards have higher chance of going over time" };
		currentCommand->function = commandFindBest;

		currentCommand->argNames = makeCharList((char**)stackArgNames, currentCommand->numArgs);
		currentCommand->argDescriptions = makeCharList((char**)stackArgDescriptions, currentCommand->numArgs);
		++currentCommand;
	}

	currentCommand->name = "exit";
	currentCommand->mainDescription = "Exits the program.";
	currentCommand->numArgs = 0;
	currentCommand->function = commandExit;
	++currentCommand;

	//command line
	printf("Type \"help\" for details\n");
	for (;;)
	{
		printf(">> ");
		char stringIn[100] = {};
		fgets(stringIn, arraySize(stringIn), stdin);
		fseek(stdin, 0, SEEK_END);
		char* stringAt = stringIn;

		if (*stringAt == '\n') continue;

		//convert text to command
		char userCommand[10 * FILENAME_SIZE] = {};
		uint userCommandIndex = 0;
		char userArgs[10][FILENAME_SIZE] = {}; 
		char* userArgsPointers[10] = {};
		uint userArgsIndex = 0;

		for (; (*stringAt != ' ') && (*stringAt != '\n') && (userCommandIndex < arraySize(userCommand)); ++stringAt)
		{
			userCommand[userCommandIndex++] = *stringAt;
		}
		++stringAt;

		bool startOfWord = true;
		for (; *stringAt != 0 && userArgsIndex < 10; ++stringAt)
		{
			uint userArgIndex = 0;
			for (; ((*stringAt != ' ') && (*stringAt != '\n') && (userArgIndex < FILENAME_SIZE)); ++stringAt)
			{
				if (startOfWord)
				{
					userArgsPointers[userArgsIndex] = userArgs[userArgsIndex];
					startOfWord = false;
				}
				userArgs[userArgsIndex][userArgIndex++] = *stringAt;
			}
			startOfWord = true;
			++userArgsIndex;
		}

		char* errorCode = 0;

		for (uint i = 0; i < arraySize(commands); ++i)
		{
			Command &command = commands[i];
			if (command.name == 0) break;

			if (doWordsMatch(userCommand, command.name))
			{
				if (userArgsIndex > command.numArgs)
				{
					errorCode = "Too many arguments";
					break;
				}

				for (uint i = 0; i < userArgsIndex; ++i)
				{
					if (userArgsPointers[i] == 0)
					{
						errorCode = "Invalid argument input";
						break;
					}
				}
				if (errorCode) break;

				void* commandData = 0;

				if (doWordsMatch(command.name, "help"))
				{
					commandData = (void*)commands;
				}
				else if (doWordsMatch(command.name, "solve"))
				{
					commandData = (void*)&queue;
				}
				else if (doWordsMatch(command.name, "find_best"))
				{
					commandData = (void*)&queue;
				}

				bool shouldExit = command.function(userArgsIndex, userArgsPointers, errorCode, commandData);
				if (errorCode) break;
				if (shouldExit) return 0;
			}
		}
		if (errorCode)
		{
			printf("ERROR: %s\n", errorCode);
		}
	}
	return 0;
}
