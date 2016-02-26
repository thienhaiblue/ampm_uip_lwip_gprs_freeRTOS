/* ampm_ff_port.h
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



#ifndef __AMPM_FF_PORT_H__
#define __AMPM_FF_PORT_H__

#include <stdint.h>
#include "sst25.h"
#include "ampm_ff/ampm_ff.h"

#define AMPM_SEEK_SET 0
#define AMPM_SEEK_CUR 1
#define AMPM_SEEK_END 2


AMPM_FIL* ampm_fopen_win32(const char * filename, const char * mode);
AMPM_FRESULT  ampm_funlink_win32(char *fileName);
AMPM_FRESULT  ampm_fclose_win32(AMPM_FIL *fil);
AMPM_FRESULT  ampm_fwrite_win32 (
	char* ptr,		/* Pointer to data buffer */
	uint32_t size,		/* Number of bytes to read */
	uint32_t count,		
	AMPM_FIL* fil 		/* Pointer to the file object */
);
AMPM_FRESULT  ampm_fread_win32 (
	char* ptr,		/* Pointer to data buffer */
	uint32_t size,		/* Number of bytes to read */
	uint32_t count,		
	AMPM_FIL* fil 		/* Pointer to the file object */
);

AMPM_FRESULT  ampm_fseek_win32 (
	AMPM_FIL* fil, 		/* Pointer to the file object */
	uint32_t offset,		/* File pointer from top of file */
	uint8_t origin
);

#define ampm_ftell_win32(fp) ((fp)->file_offset)
#define ampm_fsize_win32(fp) ((fp)->file_size)


#endif
