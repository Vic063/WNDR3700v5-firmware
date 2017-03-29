//====================================
//
// Initial Author: Vico
// Date: 14 April 2015
// File: main.cpp
//
// Purpose: Application entry point
//
//====================================

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif
#include "packer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

bool openFile( const char* );
size_t getFileSize( void );

uint8_t* loadFile( size_t );
void closeFile( void );
void printFileHeader( sercomm_pid_t );
void verifyImage( const char *fileName );
void checksumize( const uint8_t*, size_t );
int waitBeforeQuit( int errorCode );

static FILE *fs = NULL;
static uint32_t checksum = 0;

int main( int argc, char **argv )
{
	size_t size, newSize;
	uint8_t *buffer, *newBuffer;
	char tmpFile[256], filename[256], tmpFile2[256];

	memset(filename, 0, 256);
	if (argc < 2)
	{
		puts("Usage: WNDR3700v5-Firmware <path to firmware file>");
		return waitBeforeQuit(-1);
	}

	if (!openFile(argv[1]))
	{
		printf("ERROR: Could not open file %s.\n", argv[1]);
		return waitBeforeQuit(-1);
	}
	strncpy(filename, argv[1], (strrchr(argv[1], '.') - argv[1]));

	size = getFileSize();
	buffer = loadFile(size);
	closeFile();
	if (buffer == NULL)
	{
		puts("ERROR: Could not read data from input file.");
		return waitBeforeQuit(-1);
	}

	newBuffer = prebuildBinaryFile(buffer, size, &newSize);
	if (!newBuffer)
	{
		free(buffer);
		puts("Prebuild operation failed.");
		return waitBeforeQuit(-1);
	}
	free(buffer);

	memset(tmpFile, 0, 256);
	strncpy(tmpFile, filename, strlen(filename));
	strcat(tmpFile, ".tmp");

	fs = fopen(tmpFile, "wb");
	if (!fs)
	{
		free(newBuffer);
		printf("Could not open file %s for output operations.\n", tmpFile);
		return waitBeforeQuit(-1);
	}

	fwrite(newBuffer, sizeof(uint8_t), newSize, fs);
	fclose(fs);
	free(newBuffer);

	memset(tmpFile, 0, 256);
	strncpy(tmpFile, filename, strlen(filename));
	strcat(tmpFile, ".bin.original");
	rename(argv[1], tmpFile);

	memset(tmpFile, 0, 256);
	strncpy(tmpFile, filename, strlen(filename));
	strcat(tmpFile, ".tmp");
	memset(tmpFile2, 0, 256);
	strncpy(tmpFile2, filename, strlen(filename));
	strcat(tmpFile2, ".bin");

	rename(tmpFile, tmpFile2);
	/*finalizeBuild(tmpFile2);*/

	puts("Firmware creation succeded!");
	/*memset(tmpFile, 0, 256);
	strncpy(tmpFile, filename, strlen(filename));
	strcat(tmpFile, ".img");
	verifyImage(tmpFile);

	remove(tmpFile2);
	memset(tmpFile, 0, 256);
	strncpy(tmpFile, filename, strlen(filename));
	strcat(tmpFile, ".bin.tmp");
	rename(tmpFile, argv[1]);*/

	return waitBeforeQuit(0);
}

bool openFile( const char *pathToFile )
{
	fs = fopen(pathToFile, "rb");
	if (fs == NULL)
		return false;

	return true;
}

size_t getFileSize( void )
{
	size_t size;

	fseek(fs, 0, SEEK_END);
	size = ftell(fs);
	fseek(fs, 0, SEEK_SET);

	return size;
}

uint8_t* loadFile( size_t size )
{
	uint8_t *buffer;
	size_t read;

	buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
	if (buffer != NULL)
	{
		read = fread(buffer, sizeof(uint8_t), size, fs);
		if (read != size)
			printf("WARNING: Expected: %lu bytes, read: %lu bytes\n", size, read);
	}

	return buffer;
}

void closeFile( void )
{
	fclose(fs);
}

void printFileHeader( sercomm_pid_t sercomm_pid )
{
	uint16_t ver_control, download, protocol_id, hw_ver, start, c_size;

	memcpy(&ver_control, sercomm_pid.ver_control, sizeof(uint16_t));
	ver_control = htons(ver_control);
	memcpy(&download, sercomm_pid.download, sizeof(uint16_t));
	download = htons(download);
	memcpy(&hw_ver, sercomm_pid.hw_ver, sizeof(uint16_t));
	hw_ver = htons(hw_ver);
	memcpy(&protocol_id, sercomm_pid.protocol_id, sizeof(uint16_t));
	protocol_id = htons(protocol_id);
	memcpy(&start, sercomm_pid.start, sizeof(uint16_t));
	start = htons(start);
	memcpy(&c_size, sercomm_pid.c_size, sizeof(uint16_t));
	c_size = htons(c_size);

	printf("MAGIC START: %s\n", sercomm_pid.magic_s);
	printf("Version control: %d\n", ver_control);
	printf("Donwload control: %d\n", download);
	printf("Hardware ID: %s\n", sercomm_pid.hw_id);
	printf("Hardware version: %d\n", hw_ver);
	printf("Product ID: %s\n", sercomm_pid.p_id);
	printf("Protocol ID: %02x\n", protocol_id);
	printf("Function ID: %02x %02x %02x %02x %02x %02x\n", sercomm_pid.fun_id[0], sercomm_pid.fun_id[1], sercomm_pid.fun_id[2], sercomm_pid.fun_id[3], sercomm_pid.fun_id[4], sercomm_pid.fun_id[5]);
	printf("Firmware version: %d.%02x\n", sercomm_pid.fw_ver[0], sercomm_pid.fw_ver[1]);
	printf("Code segment start: %d\n", start);
	printf("Code size: %d\n", c_size);
	printf("MAGIC END: %s\n", sercomm_pid.magic_e);
}

void verifyImage( const char *fileName )
{
	sercomm_pid_t sercomm_pid;
	size_t size;
	uint8_t *buffer;

	if (!openFile(fileName))
	{
		puts("ERROR: Could not open file.");
		return;
	}

	size = getFileSize();
	buffer = loadFile(size);
	closeFile();
	if (buffer == NULL)
	{
		puts("ERROR: Could not read data from input file.");
		return;
	}

	memset(&sercomm_pid, 0, sizeof(sercomm_pid_t));
	memcpy(&sercomm_pid, buffer, sizeof(sercomm_pid_t));
	for (uint32_t i = 0; i < size / 1024; ++i)
		checksumize(buffer, 1024);
	checksumize(buffer, size % 1024);
	free(buffer);

	printFileHeader(sercomm_pid);
	printf("Firmware checksum is %08x\n", checksum);
	if (checksum != 0)
		puts("Invalid firmware checksum!");
	else
		puts("Firmware checksum is valid!");
}

void checksumize( const uint8_t *buffer, size_t length )
{
	for (size_t i = 0; i < length; ++i)
	{
		checksum += buffer[i];
		checksum <<= 24;
	}
	checksum >>= 24;
}

int waitBeforeQuit( int errorCode )
{
	puts("Press any key to close this window...");
	getchar();
	getchar();

	return errorCode;
}
