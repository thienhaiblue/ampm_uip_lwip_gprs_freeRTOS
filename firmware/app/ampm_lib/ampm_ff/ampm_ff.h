/* ampm_ff.h
 *
 * Copyright (C) AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
 * Email: thienhaiblue@ampm.com.vn or thienhaiblue@mail.com
 *
 * This file is part of ampm_open_lib
 *
 * ampm_open_lib is free software; you can redistribute it and/or modify.
 *
 * Add: 143/36/10 , Lien khu 5-6 street, Binh Hung Hoa B Ward, Binh Tan District , HCM City,Vietnam
 */



#ifndef __AMPM_FF_H__
#define __AMPM_FF_H__

#include <stdint.h>
#include "sst25.h"

/*
* log db layout:
* year_month/mday/hour.log		speed log every second
*/


typedef enum {
	FR_OK = 0,				/* (0) Succeeded */
	FR_ERR
} AMPM_FRESULT;


#define	AMPM_FA_READ				0x01
#define	AMPM_FA_OPEN_EXISTING	0x80
#define	AMPM_FA_WRITE			0x02
#define	AMPM_FA_CREATE_NEW		0x04
#define	AMPM_FA_CREATE_ALWAYS	0x08
#define	AMPM_FA_OPEN_ALWAYS		0x10
#define AMPM_FA_WRITTEN			0x20
#define AMPM_FA_DIRTY			0x40

#define AMPM_FA_STATUS_OK	0xa5a5a5a5
#define AMPM_FA_STATUS_ERR	0x5a5a5a5a


#define AMPM_FLASH_START_ADDR		0x00092000
#define AMPM_FLASH_SIZE_MAX			0x00100000 //1M
#define AMPM_FLASH_SECTOR_SIZE		0x1000
#define FILE_HEADER_SIZE 256
#define DISK_SECTOR_SIZE	(AMPM_FLASH_SECTOR_SIZE - FILE_HEADER_SIZE)
#define DISK_SECTOR_MAX ((AMPM_FLASH_SIZE_MAX - AMPM_FLASH_START_ADDR) / AMPM_FLASH_SECTOR_SIZE)

//if start_addr == previous_sector mean this is first sector of this file
typedef struct __attribute__((packed)){
	char file_name[32]; //
	uint32_t start_sector;
	uint32_t previous_sector;
	uint32_t current_sector;
	uint32_t next_sector;
	uint32_t file_status;
	uint32_t file_offset;
	uint32_t permission;
	uint32_t file_size;
	uint32_t crc;
}AMPM_FIL;




AMPM_FRESULT  ampm_f_init(void);
AMPM_FRESULT  ampm_flash_write(uint32_t addr, const uint8_t *buf, uint32_t count);
AMPM_FRESULT  ampm_flash_read(uint32_t addr, const uint8_t *buf, uint32_t count);
AMPM_FRESULT  ampm_f_printf(AMPM_FIL *fil,const uint8_t *format, ...);
AMPM_FRESULT  ampm_f_open(AMPM_FIL *fil,char  *fileName,uint8_t mode);
uint32_t  ampm_f_getfree(void);
AMPM_FRESULT  ampm_f_unlink(char *fileName);
AMPM_FRESULT  ampm_f_close(AMPM_FIL *fil);
AMPM_FRESULT  ampm_f_printf(AMPM_FIL *fil,const uint8_t *format, ...);
AMPM_FRESULT  ampm_f_write (
	AMPM_FIL* fil, 		/* Pointer to the file object */
	char* buff,		/* Pointer to data buffer */
	uint32_t btr,		/* Number of bytes to read */
	uint32_t* br		/* Pointer to number of bytes read */
);
AMPM_FRESULT  ampm_f_read (
	AMPM_FIL* fil, 		/* Pointer to the file object */
	char* buff,		/* Pointer to data buffer */
	uint32_t btr,		/* Number of bytes to read */
	uint32_t* br		/* Pointer to number of bytes read */
);
AMPM_FRESULT  ampm_f_searchfile(AMPM_FIL *fil);
AMPM_FRESULT  ampm_f_lseek (
	AMPM_FIL* fil, 		/* Pointer to the file object */
	uint32_t seek		/* File pointer from top of file */
);
AMPM_FRESULT  ampm_f_disk_cleanup(void);
AMPM_FRESULT  ampm_f_disk_format(void);
#define ampm_f_tell(fp) ((fp)->file_offset)
#define ampm_f_size(fp) ((fp)->file_size)

#endif

