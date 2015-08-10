//====================================
//
// Initial Author: Vico
// Date: 17 April 2015
// File: packer.h
//
// Purpose: defines functions to build 
//			a Sercomm firmware
//
//====================================

#ifndef _PACKER_H
#define _PACKER_H

#include <stdint.h>
#include "upgrade_flash.h"

#define UIMAGE_HEADER_START 0x50000
#define ROOTFS_OFFSET		0x2D0000
#define SERCOMM_PID_START 0x10
#ifndef _WIN32
#define MAX_PATH 260
#endif

int verifyBinaryFile( const uint8_t* );
int initializeSercommPID( sercomm_pid_t* );
uint8_t* prebuildBinaryFile( const uint8_t*, const size_t, size_t* );
void finalizeBuild( const char* );

#endif /* PACKER_H */