/* This piece of code can be found at http://www.isysop.com/unpacking-and-repacking-u-boot-uimage-files/ */

#include <stdint.h>

#define IH_MAGIC    0x27051956    /* Image Magic Number     */
#define IH_NMLEN    32            /* Image Name Length      */

typedef struct image_header {
    uint32_t    ih_magic;         /* Image Header Magic Number */
    uint32_t    ih_hcrc;          /* Image Header CRC Checksum */
    uint32_t    ih_time;          /* Image Creation Timestamp  */
    uint32_t    ih_size;          /* Image Data Size           */
    uint32_t    ih_load;          /* Data     Load  Address    */
    uint32_t    ih_ep;            /* Entry Point Address       */
    uint32_t    ih_dcrc;          /* Image Data CRC Checksum   */
    uint8_t     ih_os;            /* Operating System          */
    uint8_t     ih_arch;          /* CPU architecture          */
    uint8_t     ih_type;          /* Image Type                */
    uint8_t     ih_comp;          /* Compression Type          */
    uint8_t     ih_name[IH_NMLEN];    /* Image Name            */
} image_header_t;