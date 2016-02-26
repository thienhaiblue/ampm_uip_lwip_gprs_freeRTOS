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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ampm_ff/ampm_ff.h"
#include "ampm_ff/ampm_ff_port.h"
/*WIN32 START*/
AMPM_FIL*  ampm_fopen_win32(const char * filename, const char * mode)
{
	AMPM_FIL *file;
	AMPM_FRESULT res;
	file = malloc(sizeof(AMPM_FIL));
	if(mode[0] == 'r')
		res = ampm_f_open(file,(char *)filename,AMPM_FA_READ);
	else if(mode[0] == 'w')
		res = ampm_f_open(file,(char *)filename,AMPM_FA_CREATE_ALWAYS);
	else if(mode[0] == 'a')
	{
		res = ampm_f_open(file,(char *)filename,AMPM_FA_OPEN_ALWAYS | AMPM_FA_READ | AMPM_FA_WRITE);
		if(res == FR_OK)
		{
			ampm_f_lseek(file,file->file_size);
		}
	}
	if(res != FR_OK)
	{
		free(file);
		return 0;
	}
	return file;
}


AMPM_FRESULT  ampm_fread_win32 (
	char* ptr,		/* Pointer to data buffer */
	uint32_t size,		/* Number of bytes to read */
	uint32_t count,		
	AMPM_FIL* fil 		/* Pointer to the file object */
)
{
	uint32_t bytes;
	return ampm_f_read(fil,ptr,size,&bytes);
}


AMPM_FRESULT  ampm_fwrite_win32 (
	char* ptr,		/* Pointer to data buffer */
	uint32_t size,		/* Number of bytes to read */
	uint32_t count,		
	AMPM_FIL* fil 		/* Pointer to the file object */
)
{
	uint32_t bytes;
	return ampm_f_write(fil,ptr,size,&bytes);
}


AMPM_FRESULT  ampm_fseek_win32 (
	AMPM_FIL* fil, 		/* Pointer to the file object */
	uint32_t offset,		/* File pointer from top of file */
	uint8_t origin
)
{
	if(origin == AMPM_SEEK_SET)
	{
		return ampm_f_lseek(fil,0);
	}
	else if(origin == AMPM_SEEK_END)
	{
		return ampm_f_lseek(fil,fil->file_size);
	}
	return FR_ERR;
}

AMPM_FRESULT  ampm_fclose_win32 (
	AMPM_FIL* fil 		/* Pointer to the file object */
)
{
	AMPM_FRESULT res;
	res = ampm_f_close(fil);
	free(fil);
	return res;
}
;

