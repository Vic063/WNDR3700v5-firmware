/***************************************************************
 *             Copyright (C) 2003 by SerComm Corp.
 *                    All Rights Reserved.
 *
 *      Use of this software is restricted to the terms and
 *      conditions of SerComm's software license agreement.
 *
 *                        www.sercomm.com
 ****************************************************************/
#ifndef _UPGRADE_FLASH_H_
#define _UPGRADE_FLASH_H_

//#include "sc_project_defines.h"

#define FILESYSTEM	    "/dev/mtdblock3"
#define KERNEL_PATH	    "/dev/mtdblock2"
#define LOADER	 	    "/dev/mtdblock0"
#define SC_PID		    "/dev/mtdblock1"
#define ENGLISH_LANGUAGE	"/dev/mtdblock4"
#define SC_PRIVATE	    "/dev/mtdblock11"
#define WIFI_DATA	    "/dev/mtdblock10"

#define MAGIC_SIGN      "sErCoMm"

#define UPGRADE_TIMEOUT 15
//typedef unsigned long UINT32;
//#define ERROR_REPORT(fmt, args...)  SC_XCFPRINTF("\033[31m" fmt "\033[0m", ##args)
//#define LOG_REPORT(fmt, args...)    SC_XCFPRINTF("\033[32m" fmt "\033[0m", ##args)
#define TMP_FW          "/tmp/fw.img"
#define UPGRADING_FILE	"/tmp/upgrading"

#define PID_LEN		    70
#define MIN_UPLOAD_LEN  (1024*1024)

#ifdef FLASH_4M
#define FLASH_SIZE      (4096*1024)
#endif

#ifdef FLASH_8M
#define FLASH_SIZE      (8192*1024)
#endif

#ifdef FLASH_16M
#define FLASH_SIZE      (16384*1024)
/* the offset in the image */
#define KERNEL_OFFSET	0x050000
#define KERNEL_SIZE		0x280000
#define FS_OFFSET		0x2D0000
#define FS_SIZE			0xBA0000
#define English_OFFSET	0xE70000
#define English_SIZE	0x020000
#endif

#ifdef FLASH_128M
//#define FLASH_SIZE      (128*1014*1024)
#define FLASH_SIZE      (0x8000000)
/* the offset in the image */
#define KERNEL_OFFSET	0x200000
#define KERNEL_SIZE		0x400000
#define FS_OFFSET		0x600000
#define FS_SIZE			0x1C00000
//#define KERNEL_FS_SIZE   (0xF00000 - KERNEL_FS_OFFSET - 0x5 * (64<<10) - 0x6 * (128<<10))
//#define KERNEL_FS_SIZE   0xF00000
#define English_OFFSET		0x2200000
#define English_SIZE		0x200000

#endif

#define PID_OFFSET	   SC_BOOTLOADER_PID_END - PID_LEN // offset in mtd block

/* The assigned parameters below in SC_PRIVATA_DATA block */
#define MAC_OFFSET	    0x0B0
#define MAC_LEN         6
#define SN_OFFSET       0x0B7
#define SN_LEN          42
#define WPSPIN_OFFSET   0x0E1
#define WPSPIN_LEN      12
#define LANG_ID_OFFSET  0x0EE
#define LANG_ID_LEN     4
#define DOMAIN_OFFSET   0x0F2
#define DOMAIN_LEN      4
#define PCBA_SN_OFFSET  0x0F6
#define PCBA_SN_LEN     12

#define REMOTESCFGMGRMAGIC_OFFSET  0x102
#define REMOTESCFGMGRMAGIC_LEN     4

#define SSID_OFFSET  0x106
#define SSID_LEN     20
#define PHRASE_OFFSET  0x120
#define PHRASE_LEN     63
#define PRODUCT_ID_OFFSET 0x159
#define PRODUCT_ID_LEN 4

#define MODULE_ID_OFFSET      0x200
#define MODULE_ID_LEN 2

#define CAL_OFFSET      0x10000
#define CAL_LEN         5

#ifdef MT_CODE
/* This byte is in the MP img, so offset should remove BT length. */
#define SWITCH_MODE_OFFSET 0x39FFE0 - KERNEL_FS_OFFSET
#define SWITCH_MODE_LEN 1
#endif

typedef struct sercomm_pid
{
    unsigned char	magic_s[7];	/* sErCoMm */
    unsigned char	ver_control[2];	/* version control */
    unsigned char	download[2];	/* download control */
    unsigned char	hw_id[32];  	/* H/W ID */
    unsigned char	hw_ver[2];  	/* H/W version */
    unsigned char	p_id[4];    	/* Product ID */
    unsigned char	protocol_id[2];	/* Protocol ID */
    unsigned char	fun_id[6];	/* Function ID */
    unsigned char	fw_ver[2];	/* F/W version */
    unsigned char	start[2];	/* Starting code segment */
    unsigned char	c_size[2];	/* Code size (kbytes) */
    unsigned char	magic_e[7];	/* sErCoMm */
}sercomm_pid_t;

int is_downgrade(char *file);
void show_and_exit(int index);
int image_is_correct(char *file);
void gw_fw_update(char *img);
#endif /* _UPGRADE_FLASH_H_ */

