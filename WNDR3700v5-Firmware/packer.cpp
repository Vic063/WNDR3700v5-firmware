//====================================
//
// Initial Author: Vico
// Date: 17 April 2015
// File: packer.cpp
//
//====================================

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif
#include "packer.h"
#include "uImage.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int verifyBinaryFile( const uint8_t *buffer )
{
	image_header_t image_header;

	memcpy(&image_header, buffer, sizeof(image_header_t));
	if (image_header.ih_magic != ntohl(IH_MAGIC))
	{
		printf("uImage magic number verification failed. Got %08x\n", image_header.ih_magic);
		return -1;
	}

	printf("Using uImage %s\n", image_header.ih_name);
	return 0;
}

int initializeSercommPID( sercomm_pid_t *sercomm_pid )
{
	char fw[6], *pEnd;
	uint32_t fw_major, fw_minor, fw_micro;
	int err;

	memset(sercomm_pid, 0, sizeof(sercomm_pid_t));
	memcpy(sercomm_pid->magic_s, "sErCoMm", 7);
	sercomm_pid->ver_control[1] = 1;
	memcpy(sercomm_pid->hw_id, "AYB", 3);
	memcpy(sercomm_pid->p_id, "A001", 4);
	memcpy(sercomm_pid->magic_e, "sErCoMm", 7);

	puts("Please enter the firmware version (ex: 1.3.37):");
	scanf("%s", fw);

	err = sscanf(fw, "%d.%d.%d", &fw_major, &fw_minor, &fw_micro);
	if (err != 3)
	{
		puts("Bad firmware version.");
		return -1;
	}

	sprintf(fw, "0x%1d%1d", fw_major, fw_minor);
	sercomm_pid->fw_ver[0] = strtol(fw, &pEnd, 16) & 0xFF;
	sprintf(fw, "0x%02d", fw_micro);
	sercomm_pid->fw_ver[1] = strtol(fw, &pEnd, 16) & 0xFF;

	return 0;
}

uint8_t* prebuildBinaryFile( const uint8_t *buffer, const size_t length, size_t *newSize )
{
	uint8_t *newBuffer;
	sercomm_pid_t sercomm_pid;
	image_header_t image_header;
	uint32_t size;

	if (verifyBinaryFile(buffer) != 0)
	{
		puts("uImage verification failed.");
		return NULL;
	}

	if (initializeSercommPID(&sercomm_pid) == -1)
	{
		puts("Sercomm PID initialization failed.");
		return NULL;
	}
	
	memcpy(&image_header, buffer, sizeof(image_header_t));
	*newSize = ROOTFS_OFFSET + length - ntohl(image_header.ih_size) - 4;
	while ((*newSize % 16 ) != 0)
		*newSize += 1;

	newBuffer = (uint8_t*)malloc(*newSize);
	if (!newBuffer)
	{
		printf("Could not allocate memory of %u bytes to prebuild file.\n", size);
		return NULL;
	}

	//Populate the whole buffer with 0xFF
	memset(newBuffer, 0xFF, *newSize);
	memcpy((newBuffer + SERCOMM_PID_START), &sercomm_pid, sizeof(sercomm_pid_t));

	//Copy the uImage header
	memcpy((newBuffer + UIMAGE_HEADER_START), buffer, sizeof(image_header_t));

	//Copy the uImage data
	memcpy((newBuffer + UIMAGE_HEADER_START + sizeof(image_header_t)), (buffer + sizeof(image_header_t)), ntohl(image_header.ih_size));

	//Copy the rootfs data
	memcpy((newBuffer + ROOTFS_OFFSET), (buffer + sizeof(image_header_t) + ntohl(image_header.ih_size)), (length - (sizeof(image_header_t) + ntohl(image_header.ih_size) + 4)));

	return newBuffer;
}

void finalizeBuild( const char *fileName )
{
	char cmdline[MAX_PATH + 50];
#ifdef _WIN32
	DWORD bytesWritten;
	char *directory, *path;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
#else
	int err;
#endif

#ifndef _WIN32
	sprintf(cmdline, "./zipImage %s", fileName);
	err = system(cmdline);
#else
	directory = (char*)calloc(sizeof(char), MAX_PATH);
	if (directory == NULL)
	{
		puts("Could not allocate memory for directory.");
		return;
	}

	bytesWritten = GetModuleFileName(NULL, directory, MAX_PATH);
	if (bytesWritten == 0)
	{
		free(directory);
		puts("Could not get current directory.");
		return;
	}
	
	path = (char*)calloc(sizeof(char), MAX_PATH);
	if (path == NULL)
	{
		free(directory);
		puts("Could not allocate memory for path.");
		return;
	}

	strncpy(path, directory, (strrchr(directory, '\\') - directory));
	sprintf(cmdline, "\"%s\\zipImage.exe\" \"%s\"", path, fileName);
	free(directory);
	free(path);

	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(si);

	if (!CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		puts("Unable to start zipImage process.");
	else
	{
		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
#endif
}
