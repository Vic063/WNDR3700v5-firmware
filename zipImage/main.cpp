//====================================
//
// Initial Author: Vico
// Date: 20 April 2015
// File: main.cpp
//
// Purpose: Application entry point
//
//====================================

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>

uint8_t* findPid( const uint8_t*, const size_t );
uint8_t calculatePidChecksum( const uint8_t* , const uint8_t*, const size_t );
uint8_t calculateFileChecksum( const char* );
int waitBeforeQuit( int errorCode );

int main( int argc, char **argv )
{
	char command[512];
	char filename[256], tmpFile[256];
	FILE *fsZip, *fsBin, *fsImg;
	uint8_t *bufferZip, *bufferBin, *pid, pidChecksum;
	size_t sizeZip, sizeBin;
	DWORD bytesWritten;
	char *directory, *path;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if (argc < 2)
	{
		puts("Usage: zipImage <binfile>");
		return waitBeforeQuit(-1);
	}

	//Get the filename without extension
	memset(filename, 0, 256);
	strncpy(filename, argv[1], (strrchr(argv[1], '.') - argv[1]));

	//Zip the *.bin file
	directory = (char*)calloc(sizeof(char), MAX_PATH);
	if (directory == NULL)
	{
		puts("Could not allocate memory for directory.");
		return waitBeforeQuit(-1);
	}

	bytesWritten = GetModuleFileName(NULL, directory, MAX_PATH);
	if (bytesWritten == 0)
	{
		free(directory);
		puts("Could not get current directory.");
		return waitBeforeQuit(-1);
	}
	
	path = (char*)calloc(sizeof(char), MAX_PATH);
	if (path == NULL)
	{
		free(directory);
		puts("Could not allocate memory for path.");
		return waitBeforeQuit(-1);
	}

	strncpy(path, directory, (strrchr(directory, '\\') - directory));
	sprintf(command, "%s\\zip.exe -j \"%s.zip\" \"%s\"", path, filename, argv[1]);
	free(directory);
	free(path);

	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(si);

	if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		puts("Unable to start zipImage process.");
		return waitBeforeQuit(-1);
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

	//Open the zip file
	memset(tmpFile, 0, 256);
	strncpy(tmpFile, filename, strlen(filename));
	strcat(tmpFile, ".zip");
	fsZip = fopen(tmpFile, "rb");
	if (fsZip == NULL)
	{
		puts("The specified zip file can not be opened!");
		return waitBeforeQuit(-1);
	}

	//Get the Zip size
	fseek(fsZip, 0, SEEK_END);
	sizeZip = ftell(fsZip);
	fseek(fsZip, 0, SEEK_SET);

	//Allocate a buffer to contain the content of the Zip file
	bufferZip = (uint8_t*)malloc(sizeZip * sizeof(uint8_t));
	if (bufferZip == NULL)
	{
		fclose(fsZip);
		printf("Error, no enough memory to open the specified file: %s\n", tmpFile);
		return waitBeforeQuit(-1);
	}

	//Read the whole Zip into the buffer
	fread(bufferZip, sizeof(uint8_t), sizeZip, fsZip);
	fclose(fsZip);

	//Remove the Zip file
	//remove(tmpFile);

	//Open the bin file
	fsBin = fopen(argv[1], "rb");
	if (fsBin == NULL)
	{
		puts("The specified bin file can not be opened!");
		free(bufferZip);
		return waitBeforeQuit(-1);
	}

	//Get the Bin size
	fseek(fsBin, 0, SEEK_END);
	sizeBin = ftell(fsBin);
	fseek(fsBin, 0, SEEK_SET);

	//Allocate a buffer to contain the content of the Bin file
	bufferBin = (uint8_t*)malloc(sizeBin * sizeof(uint8_t));
	if (bufferBin == NULL)
	{
		free(bufferZip);
		fclose(fsBin);
		printf("Error, no enough memory to open the specified file: %s\n", tmpFile);
		return waitBeforeQuit(-1);
	}

	//Read the whole Bin into the buffer
	fread(bufferBin, sizeof(uint8_t), sizeBin, fsBin);
	fclose(fsBin);

	//Try to find the pid
	if ((pid = findPid(bufferBin, sizeBin)) == NULL)
	{
		free(bufferBin);
		free(bufferZip);
		return waitBeforeQuit(-1);
	}

	pidChecksum = calculatePidChecksum(pid, bufferZip, sizeZip);
	printf("Pid Checksum after neg: %#02x\n", pidChecksum);
	pid[0x1FF] = pidChecksum;

	//Save everything as an *.img file
	memset(tmpFile, 0, 256);
	strncpy(tmpFile, filename, strlen(filename));
	strcat(tmpFile, ".img");
	fsImg = fopen(tmpFile, "wb");
	if (fsImg == NULL)
	{
		free(bufferBin);
		free(bufferZip);
		free(pid);
		puts("The specified img file could not be opened!");
		return waitBeforeQuit(-1);
	}

	//Write the pid
	fwrite(pid, sizeof(uint8_t), 0x200, fsImg);
	//Write the Zip content
	fwrite(bufferZip, sizeof(uint8_t), sizeZip, fsImg);
	fclose(fsImg);

	free(bufferZip);
	free(bufferBin);
	free(pid);

	if (calculateFileChecksum(tmpFile) == 0)
		printf("Success to build %s img!\n", filename);
	else
		puts("Error: Checksum fail!");

	return waitBeforeQuit(0);
}

uint8_t* findPid( const uint8_t *binBuffer, const size_t binSize )
{
	uint8_t hasFoundStart = 0;
	uint8_t *pid;
	uint32_t start, end;
	size_t i = 0;

	for (; i < binSize; ++i)
	{
		if (binBuffer[i] == 0x73)
		{
			if (strncmp((const char*)(binBuffer + i), "sErCoMm", 7) == 0 && !hasFoundStart)
			{
				printf("The first magic sign is found at offset %#lx\n", i);
				hasFoundStart = 1;
				start = i;
				i += 7;
			}

			if (strncmp((const char*)(binBuffer + i), "sErCoMm", 7) == 0 && hasFoundStart)
			{
				printf("The second magic sign is found at offset %#lx\n", i);
				i += 7;
				end = i;
				hasFoundStart = 2;
				break;
			}
		}
	}

	if (hasFoundStart != 2)
	{
		puts("Error, fail to get pid from bin file.");
		return NULL;
	}

	if (end - start > 0x46)
	{
		printf("Error, pid len %d is greater than %d\n", end - start, 0x46);
		return NULL;
	}

	pid = (uint8_t*)calloc(0x200, sizeof(uint8_t));
	if (pid == NULL)
	{
		puts("Failed to allocate memory for pid.");
		return NULL;
	}

	memcpy(pid, binBuffer + start, 0x46);
	return pid;
}

uint8_t calculatePidChecksum( const uint8_t *pid , const uint8_t *zipBuffer, const size_t zipSize )
{
	uint8_t checksum = 0;
	size_t i = 0, j = 0;

	for (; i < 0x200; ++i)
		checksum += pid[i];

	for (; j < zipSize; ++j)
		checksum += zipBuffer[j];

	printf("Pid Checksum before neg: %#02x\n", checksum);

	return ~checksum;
}

uint8_t calculateFileChecksum( const char *filename )
{
	FILE *fs;
	uint8_t buffer[1], checksum = 0;

	fs = fopen(filename, "rb");
	if (fs == NULL)
	{
		printf("Could not open file %s to verify its checksum.\n", filename);
		return -1;
	}

	while (fread(buffer, sizeof(uint8_t), 1, fs) != 0)
		checksum += buffer[0];
	fclose(fs);

	return checksum;
}

int waitBeforeQuit( int errorCode )
{
	/*puts("Press any key to close this window...");
	getchar();*/

	return errorCode;
}
