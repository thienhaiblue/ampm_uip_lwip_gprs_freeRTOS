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

#define AMPM_FF_DBG(...)		

uint8_t ampm_file_buf[256];


AMPM_FRESULT  ampm_f_init(void)
{
	if(SST25_Init() == SST25_OK)
	{
		ampm_f_disk_cleanup();
		return FR_OK;
	}
	return FR_ERR;
}

static uint32_t  ampm_ff_checksum(uint8_t *buff, uint16_t length)
{
	uint32_t i;
	uint32_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}

static AMPM_FRESULT  ampm_flash_erase_sector(uint32_t addr)
{
	if(SST25_Erase(addr,block4k) == SST25_OK)
		return FR_OK;
	return FR_ERR;
}

AMPM_FRESULT  ampm_flash_read(uint32_t addr, const uint8_t *buf, uint32_t count)
{
	if(SST25_Read(addr,(uint8_t *)buf,count) == SST25_OK)
	{
		return FR_OK;
	}
	return FR_ERR;
}

AMPM_FRESULT  ampm_flash_write(uint32_t addr, const uint8_t *buf, uint32_t count)
{
	if(SST25_Write(addr,buf,count) == SST25_OK)
		return FR_OK;
	else
		return FR_ERR;
}

AMPM_FRESULT  _ampm_f_file_format_check(AMPM_FIL *fil)
{
	uint32_t crc;
	uint8_t buf[32];
	if( (fil == NULL) )
	{
		return FR_ERR;
	}
	memcpy(buf,fil->file_name,sizeof(fil->file_name));
	buf[31] = 0;
	crc = ampm_ff_checksum((uint8_t *)fil,(sizeof(AMPM_FIL)-sizeof(fil->crc)));
	if(fil->file_name[0] 
		&& (fil->start_sector < DISK_SECTOR_MAX)
		&& (fil->current_sector < DISK_SECTOR_MAX)
		&& (fil->previous_sector < DISK_SECTOR_MAX)
		&& (fil->crc == crc) 
		&& (fil->file_status == AMPM_FA_STATUS_OK) 
		&& (fil->file_offset <= fil->file_size)
		&& (fil->file_size < 100000)
		&& (fil->permission <= 0xff)
		&& (strlen((char *)buf) < (sizeof(fil->file_name) - 3)) 
	)
	{
		return FR_OK;
	}
	return FR_ERR;
}

AMPM_FRESULT  ampm_f_file_format_check(AMPM_FIL *fil)
{
	if(_ampm_f_file_format_check(fil) == FR_OK)
	{
		 ampm_flash_read(AMPM_FLASH_START_ADDR + fil->start_sector*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)fil,sizeof(AMPM_FIL));
		if(_ampm_f_file_format_check(fil) == FR_OK)
		{
			return FR_OK;
		}
	}
	return FR_ERR;
}

AMPM_FRESULT  ampm_f_disk_format(void)
{
	uint32_t i;
	for(i = 0; i < DISK_SECTOR_MAX; i++)
	{
		ampm_flash_erase_sector(AMPM_FLASH_START_ADDR + i*AMPM_FLASH_SECTOR_SIZE);
	}
	return FR_OK;
}


AMPM_FRESULT  ampm_f_disk_cleanup(void)//clear all err file
{
	AMPM_FIL fil;
	uint32_t i,start_sector;
	for(i = 0; i < DISK_SECTOR_MAX; i++)
	{
		 ampm_flash_read(AMPM_FLASH_START_ADDR + i*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)&fil,sizeof(AMPM_FIL));
		if(ampm_f_file_format_check(&fil) == FR_OK)
		{
			start_sector = fil.start_sector;
			 ampm_flash_read(AMPM_FLASH_START_ADDR + fil.start_sector*AMPM_FLASH_SECTOR_SIZE ,(uint8_t *)&fil,sizeof(AMPM_FIL));
			if(_ampm_f_file_format_check(&fil) == FR_OK)
			{
				 ampm_flash_read(AMPM_FLASH_START_ADDR + fil.start_sector*AMPM_FLASH_SECTOR_SIZE + 2*sizeof(AMPM_FIL),(uint8_t *)&fil,sizeof(AMPM_FIL));
				if(_ampm_f_file_format_check(&fil) == FR_ERR)
				{
					ampm_flash_erase_sector(AMPM_FLASH_START_ADDR + start_sector*AMPM_FLASH_SECTOR_SIZE);
				}
			}
		}
	}
	return FR_OK;
}


uint32_t rand_sec;

AMPM_FRESULT  ampm_f_open(AMPM_FIL *fil,char  *fileName,uint8_t mode)
{
	uint32_t i,temp;
	AMPM_FF_DBG("\r\n ampm_f_open:%s\r\n",fileName);
	AMPM_FF_DBG("\r\n FIL:0x%08X\r\n",fil);
	if( (fil == NULL) )
	{
		return FR_ERR;
	}
	//srand(SysTick_Get());
	rand_sec = rand();
	rand_sec %= DISK_SECTOR_MAX;
	temp = rand_sec;
	//AMPM_FF_DBG("\r\n ampm_f_open:rand_sec=%d\r\n",rand_sec);
	for(i = rand_sec; i < DISK_SECTOR_MAX;)
	{
		if(rand_sec)
		{
			i--;
			rand_sec--;
		}
		else
		{
			if(temp)
			{
				i = temp;
				temp = 0;
			}
			i++;
		}
		ampm_flash_read(AMPM_FLASH_START_ADDR + i*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)fil,sizeof(AMPM_FIL));
		if(ampm_f_file_format_check(fil) == FR_OK)
		{
			//AMPM_FF_DBG("\r\n ampm_f_open:file exist\r\n");
			if (mode & (AMPM_FA_READ | AMPM_FA_OPEN_ALWAYS | AMPM_FA_OPEN_EXISTING) )
			{
				if(strcmp((const char* )fileName,(const char*)fil->file_name) == 0)
				{
					AMPM_FF_DBG("\r\n ampm_f_open:open->\r\n");
					ampm_flash_read(AMPM_FLASH_START_ADDR + fil->start_sector*AMPM_FLASH_SECTOR_SIZE + 2*sizeof(AMPM_FIL),(uint8_t *)fil,sizeof(AMPM_FIL));
					if(_ampm_f_file_format_check(fil) == FR_OK)
					{
						if(mode & AMPM_FA_WRITE)
							fil->permission = AMPM_FA_WRITE;
						else
							fil->permission = AMPM_FA_READ;
						fil->file_offset = 0;
						AMPM_FF_DBG("success\r\n");
						AMPM_FF_DBG("\r\n ampm_f_open:file struct err->");
						AMPM_FF_DBG("\r\n file info\r\n{");
						AMPM_FF_DBG("\r\n name:%s",fil->file_name);
						AMPM_FF_DBG("\r\n size:%d",fil->file_size);
						AMPM_FF_DBG("\r\n offse:t%d",fil->file_offset);
						AMPM_FF_DBG("\r\n start_sector:%d",fil->start_sector);
						AMPM_FF_DBG("\r\n current_sector:%d",fil->current_sector);
						AMPM_FF_DBG("\r\n next_sector:%d",fil->next_sector);
						AMPM_FF_DBG("\r\n permission:%d",fil->permission);
						AMPM_FF_DBG("\r\n file_status:0x%08X",fil->file_status);
						AMPM_FF_DBG("\r\n}\r\n");
						return FR_OK;
					}
					else
					{
						AMPM_FF_DBG("false\r\n");
					}
				}
				else
				{
					//AMPM_FF_DBG("\r\n ampm_f_open:file name not mask\r\n");
				}
			}
		}
		else
		{
			if(mode & (AMPM_FA_CREATE_ALWAYS | AMPM_FA_CREATE_NEW))
			{
				AMPM_FF_DBG("\r\n ampm_f_open:create a new file->");
				strcpy(fil->file_name,fileName);
				fil->start_sector = i;
				fil->current_sector = i;
				fil->previous_sector = i;
				fil->next_sector = 0xffffffff;
				fil->file_size = 0;
				fil->file_offset = 0;
				fil->file_status = AMPM_FA_STATUS_OK;
				fil->permission = AMPM_FA_WRITE;
				fil->crc = ampm_ff_checksum((uint8_t *)fil,(sizeof(AMPM_FIL)-sizeof(fil->crc)));
				if(ampm_flash_write(AMPM_FLASH_START_ADDR + fil->start_sector*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)fil,sizeof(AMPM_FIL)) == FR_OK)
				{
					AMPM_FF_DBG("success\r\n");
					return FR_OK;
				}
				else
				{
					AMPM_FF_DBG("false\r\n");
					return FR_ERR;
				}
			}
		}
	}
	AMPM_FF_DBG("false\r\n");
	fil->file_name[0] = 0;
	return FR_ERR;
}

AMPM_FRESULT  ampm_f_find_free_sector(uint32_t *sector)
{
	uint32_t i,temp,rend_sec;
	AMPM_FIL fil;
	rand_sec = rand();
	rand_sec %= DISK_SECTOR_MAX;
	temp = rand_sec;
	for(i = rand_sec; i < DISK_SECTOR_MAX;)
	{
		if(rand_sec)
		{
			i--;
			rand_sec--;
		}
		else
		{
			if(temp)
			{
				i = temp;
				temp = 0;
			}
			i++;
		}
		 ampm_flash_read(AMPM_FLASH_START_ADDR + i*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)&fil,sizeof(AMPM_FIL));
		if(ampm_f_file_format_check(&fil) == FR_ERR)
		{
			*sector = i;
			return FR_OK;
		}
	}
	
	return FR_ERR;
}

uint32_t  ampm_f_getfree(void)
{
	uint32_t free_sector_cnt = 0,i;
	AMPM_FIL fil;
	AMPM_FF_DBG("\r\n ampm_f_getfree();\r\n");
	for(i = 0; i < DISK_SECTOR_MAX;i++)
	{
		 ampm_flash_read(AMPM_FLASH_START_ADDR + i*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)&fil,sizeof(AMPM_FIL));
		if(ampm_f_file_format_check(&fil) == FR_ERR)
		{
			free_sector_cnt++;
		}
	}
	AMPM_FF_DBG("\r\n free_sector_cnt:%d\r\n",free_sector_cnt);
	return free_sector_cnt;
	if(free_sector_cnt)
		return FR_OK;
	else
		return FR_ERR;
}

AMPM_FRESULT  ampm_f_searchfile(AMPM_FIL *fil)
{
	uint32_t i;
	for(i = 0; i < DISK_SECTOR_MAX;i++)
	{
		 ampm_flash_read(AMPM_FLASH_START_ADDR + i*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)fil,sizeof(AMPM_FIL));
		if(ampm_f_file_format_check(fil) == FR_OK)
		{
			 ampm_flash_read(AMPM_FLASH_START_ADDR + fil->start_sector*AMPM_FLASH_SECTOR_SIZE + 2*sizeof(AMPM_FIL),(uint8_t *)fil,sizeof(AMPM_FIL));
			if(_ampm_f_file_format_check(fil) == FR_OK)
			{
				return FR_OK;
			}
		}
	}
	return FR_ERR;
}

AMPM_FRESULT  ampm_f_unlink(char *fileName)
{
	AMPM_FIL fil;
	uint32_t i,temp;
	for(i = 0; i < DISK_SECTOR_MAX;i++)
	{
		 ampm_flash_read(AMPM_FLASH_START_ADDR + i*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)&fil,sizeof(AMPM_FIL));
		if(ampm_f_file_format_check(&fil) == FR_OK)
		{
			if(strcmp((const char* )fileName,(const char* )fil.file_name) == 0)
			{
				ampm_flash_erase_sector(AMPM_FLASH_START_ADDR + fil.start_sector*AMPM_FLASH_SECTOR_SIZE);
				ampm_flash_erase_sector(AMPM_FLASH_START_ADDR + i*AMPM_FLASH_SECTOR_SIZE);
				return FR_OK;
			}
		}
	}
	return FR_ERR;
	return FR_OK;
}


AMPM_FRESULT  ampm_f_close(AMPM_FIL *fil)
{
	uint32_t temp;
	AMPM_FIL rfil;
	if( (fil == NULL)
		|| (fil->start_sector >= DISK_SECTOR_MAX)
		|| (fil->current_sector >= DISK_SECTOR_MAX)
		|| (fil->previous_sector >= DISK_SECTOR_MAX)
		|| (fil->file_name[0] == 0)
		|| (fil->file_size > 100000)
		|| (fil->file_offset > 100000)
		|| (fil->file_offset > fil->file_size)
		|| (fil->permission > AMPM_FA_WRITE)
	)
	{
		return FR_ERR;
	}
	if(fil->permission == AMPM_FA_WRITE)
	{
		temp = fil->file_size % 256;

		ampm_flash_write(AMPM_FLASH_START_ADDR + fil->current_sector * AMPM_FLASH_SECTOR_SIZE + (fil->file_offset - temp)%DISK_SECTOR_SIZE + FILE_HEADER_SIZE,ampm_file_buf,256);
		 ampm_flash_read(AMPM_FLASH_START_ADDR + fil->start_sector*AMPM_FLASH_SECTOR_SIZE + sizeof(AMPM_FIL),(uint8_t *)&rfil,sizeof(AMPM_FIL));
		fil->current_sector = fil->start_sector;
		fil->next_sector = rfil.next_sector;
		fil->previous_sector = fil->start_sector;
		fil->crc = ampm_ff_checksum((uint8_t *)fil,(sizeof(AMPM_FIL)-sizeof(fil->crc)));
		if(ampm_flash_write(AMPM_FLASH_START_ADDR + fil->start_sector*AMPM_FLASH_SECTOR_SIZE + 2*sizeof(AMPM_FIL),(uint8_t *)fil,sizeof(AMPM_FIL)) != FR_OK)
		{
			ampm_flash_erase_sector(AMPM_FLASH_START_ADDR + fil->start_sector*AMPM_FLASH_SECTOR_SIZE);//erase file if reponse a err
			return FR_ERR;
		}
	}
	fil->file_name[0] = 0;
	return FR_OK;
}


AMPM_FRESULT  ampm_f_lseek (
	AMPM_FIL* fil, 		/* Pointer to the file object */
	uint32_t seek		/* File pointer from top of file */
)
{
	uint32_t num_sector,i,current_sector;
	AMPM_FIL rfil;
	num_sector = seek / DISK_SECTOR_SIZE;
	current_sector = fil->start_sector;
	for(i = 0;i < num_sector;i++)
	{
		 ampm_flash_read(AMPM_FLASH_START_ADDR + current_sector*AMPM_FLASH_SECTOR_SIZE + sizeof(AMPM_FIL),(uint8_t *)&rfil,sizeof(AMPM_FIL));
		if(_ampm_f_file_format_check(&rfil) == FR_ERR)
		{
			return FR_ERR;
		}
		current_sector = rfil.next_sector;
	}
	fil->current_sector = current_sector;
	fil->next_sector = rfil.next_sector;
	fil->previous_sector = rfil.previous_sector;
	fil->file_offset = seek;
	return FR_OK;
}


AMPM_FRESULT  ampm_f_next_sector (
	AMPM_FIL* fil 		/* Pointer to the file object */)
{
	AMPM_FIL rfil;
	 ampm_flash_read(AMPM_FLASH_START_ADDR + fil->start_sector*AMPM_FLASH_SECTOR_SIZE + sizeof(AMPM_FIL),(uint8_t *)&rfil,sizeof(AMPM_FIL));
	if(_ampm_f_file_format_check(&rfil) == FR_ERR)
	{
		return FR_ERR;
	}
	fil->current_sector = rfil.next_sector;
	fil->next_sector = rfil.next_sector;
	fil->previous_sector = rfil.previous_sector;
	return FR_OK;
}


AMPM_FRESULT  ampm_f_read (
	AMPM_FIL* fil, 		/* Pointer to the file object */
	char* buff,		/* Pointer to data buffer */
	uint32_t btr,		/* Number of bytes to read */
	uint32_t* br		/* Pointer to number of bytes read */
)
{
	uint32_t temp,temp0;
	if( (fil == NULL)
		|| (fil->start_sector >= DISK_SECTOR_MAX)
		|| (fil->current_sector >= DISK_SECTOR_MAX)
		|| (fil->previous_sector >= DISK_SECTOR_MAX)
		|| (fil->file_name[0] == 0)
		|| (fil->file_size > 100000)
		|| (fil->file_offset > 100000)
		|| (fil->file_offset > fil->file_size)
		|| (fil->permission > AMPM_FA_WRITE)
	)
	{
		AMPM_FF_DBG("\r\n ampm_f_read:file struct err->");
		AMPM_FF_DBG("\r\n file info\r\n{");
		AMPM_FF_DBG("\r\n name:%s",fil->file_name);
		AMPM_FF_DBG("\r\n size:%d",fil->file_size);
		AMPM_FF_DBG("\r\n offse:t%d",fil->file_offset);
		AMPM_FF_DBG("\r\n start_sector:%d",fil->start_sector);
		AMPM_FF_DBG("\r\n current_sector:%d",fil->current_sector);
		AMPM_FF_DBG("\r\n next_sector:%d",fil->next_sector);
		AMPM_FF_DBG("\r\n permission:%d",fil->permission);
		AMPM_FF_DBG("\r\n file_status:0x%08X",fil->file_status);
		AMPM_FF_DBG("\r\n}\r\n");
		return FR_ERR;
	}
	
	
	if(fil->file_offset + btr < fil->file_size)
	{
		*br = btr;
	}
	else
	{
		*br = fil->file_size - fil->file_offset;
	}
	
	temp = (fil->file_offset % DISK_SECTOR_SIZE);
	if((temp + *br) <= DISK_SECTOR_SIZE)
	{
		 ampm_flash_read(AMPM_FLASH_START_ADDR + fil->current_sector*AMPM_FLASH_SECTOR_SIZE + FILE_HEADER_SIZE + temp,(uint8_t *)buff,*br);
	}
	else
	{
		temp0 = DISK_SECTOR_SIZE - temp;
		 ampm_flash_read(AMPM_FLASH_START_ADDR + fil->current_sector*AMPM_FLASH_SECTOR_SIZE + FILE_HEADER_SIZE + temp,(uint8_t *)buff,temp0);
		temp = temp + *br;
		temp %= DISK_SECTOR_SIZE;
		if(ampm_f_next_sector(fil) == FR_ERR)
		{
			return FR_ERR;
		}
		 ampm_flash_read(AMPM_FLASH_START_ADDR + fil->current_sector*AMPM_FLASH_SECTOR_SIZE + FILE_HEADER_SIZE,(uint8_t *)&buff[temp0],temp);
	}
	fil->file_offset += *br;
	return FR_OK;
}



AMPM_FRESULT  ampm_f_write (
	AMPM_FIL* fil, 		/* Pointer to the file object */
	char* buff,		/* Pointer to data buffer */
	uint32_t btr,		/* Number of bytes to read */
	uint32_t* br		/* Pointer to number of bytes read */
)
{
	uint16_t i;
	uint32_t sector;
	if( (fil == NULL)
		|| (fil->start_sector >= DISK_SECTOR_MAX)
		|| (fil->current_sector >= DISK_SECTOR_MAX)
		|| (fil->previous_sector >= DISK_SECTOR_MAX)
		|| (fil->file_name[0] == 0)
		|| (fil->permission != AMPM_FA_WRITE)
		|| (fil->file_size > 100000)
		|| (fil->file_offset > 100000)
		|| (fil->file_offset > fil->file_size)
		|| (fil->permission > AMPM_FA_WRITE)
	)
	{
		return FR_ERR;
	}
	*br = btr;
	for(i = 0;i < btr; i++)
	{
		ampm_file_buf[fil->file_offset % 256] = buff[i];
		fil->file_size++;
		fil->file_offset++;
		if(fil->file_offset % 256 == 0)
		{
			if(ampm_flash_write(AMPM_FLASH_START_ADDR + fil->current_sector*AMPM_FLASH_SECTOR_SIZE + ((fil->file_offset - 256)%DISK_SECTOR_SIZE ) + FILE_HEADER_SIZE,ampm_file_buf,256) != FR_OK)
			{
				fil->file_status = AMPM_FA_STATUS_ERR;
				return FR_ERR;
			}
			if(fil->file_offset % DISK_SECTOR_SIZE == 0)
			{
				if(ampm_f_find_free_sector(&sector) == FR_ERR)
				{
					fil->file_status = AMPM_FA_STATUS_ERR;
					return FR_ERR;
				}
				fil->next_sector = sector;
				fil->crc = ampm_ff_checksum((uint8_t *)fil,(sizeof(AMPM_FIL)-sizeof(fil->crc)));
				if(ampm_flash_write(AMPM_FLASH_START_ADDR + fil->current_sector*AMPM_FLASH_SECTOR_SIZE + sizeof(AMPM_FIL),(uint8_t *)fil,sizeof(AMPM_FIL)) != FR_OK)
				{
					fil->file_status = AMPM_FA_STATUS_ERR;
					return FR_ERR;
				}
				fil->previous_sector = fil->current_sector;
				fil->current_sector = sector;
				fil->crc = ampm_ff_checksum((uint8_t *)fil,(sizeof(AMPM_FIL)-sizeof(fil->crc)));
				if(ampm_flash_write(AMPM_FLASH_START_ADDR + fil->current_sector*AMPM_FLASH_SECTOR_SIZE,(uint8_t *)fil,sizeof(AMPM_FIL)) != FR_OK)
				{
					fil->file_status = AMPM_FA_STATUS_ERR;
					return FR_ERR;
				}
			}
			memset(ampm_file_buf,0xff,256);
		}
	}
	return FR_OK;
}

AMPM_FRESULT  ampm_f_printf(AMPM_FIL *fil,const uint8_t *format, ...)
{
	static  uint8_t  buffer[256];
	uint32_t len,i;
	//va_list     vArgs;
	
	if( (fil == NULL)
		|| (fil->start_sector > DISK_SECTOR_MAX)
		|| (fil->file_name[0] == 0)
		|| (fil->permission != AMPM_FA_WRITE)
		|| (fil->file_size > 100000)
		|| (fil->file_offset > 100000)
		|| (fil->file_offset > fil->file_size)
		|| (fil->permission > AMPM_FA_WRITE)
	)
	{
		return FR_ERR;
	}
	/*va_start(vArgs, format);
	len = vsprintf((char *)&buffer[0], (char const *)format, vArgs);
	va_end(vArgs);*/
	len = sprintf((char *)&buffer[0], (char const *)format);
	if(len >= 256) 
	{
		len = 256;
	}
	return ampm_f_write(fil,(char *)buffer,len,&i);
}



