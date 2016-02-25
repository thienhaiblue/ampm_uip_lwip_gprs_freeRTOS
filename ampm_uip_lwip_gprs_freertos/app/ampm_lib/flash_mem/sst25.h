#ifndef _SST25_H_
#define _SST25_H_
#include <stdint.h>

#define SST25_SECTOR_SIZE	0x1000 //4096

#define FLASH_SIZE_MAX	0x00400000//4Mbytes

#define LOG_SIZE_MAX	(40*1024)
#define FIRMWARE_MAX_SIZE		0x00020000 //128k


#define BASE_FIRMWARE_INFO_ADDR 			0
#define BASE_FIRMWARE_BASE_ADDR			(0x00001000 + BASE_FIRMWARE_INFO_ADDR)

#define FIRMWARE_INFO_ADDR 								(BASE_FIRMWARE_BASE_ADDR + FIRMWARE_MAX_SIZE)
#define FIRMWARE_BASE_ADDR								(0x00001000 + FIRMWARE_INFO_ADDR)

#define PARAMETER_BASE_ADDR		(FIRMWARE_BASE_ADDR + FIRMWARE_MAX_SIZE)

#define MILEAGE_ADDR											(0x00000000 + PARAMETER_BASE_ADDR)
#define SYS_FLAG_STATUS_ADDR			(0x00001000 + PARAMETER_BASE_ADDR)
#define GPS_LAT_ADDR											(0x00002000 + PARAMETER_BASE_ADDR)
#define GPS_LON_ADDR											(0x00003000 + PARAMETER_BASE_ADDR)
#define GPS_HDOP_ADDR										(0x00004000 + PARAMETER_BASE_ADDR)
#define GPS_DIR_ADDR											(0x00005000 + PARAMETER_BASE_ADDR)
#define FIRMWARE_DOWNLOAD_TIMEOUT_ADDR					(0x00006000 + PARAMETER_BASE_ADDR)
#define LOG_POSITION_ADDR						(0x00007000 + PARAMETER_BASE_ADDR)
#define LOG_DATABASE_ADDR				(0x00008000 + PARAMETER_BASE_ADDR)
#define LOG_DATA_SIZE_MAX  LOG_SIZE_MAX


#define SST25_MSC_FLASH_START	0x00092000


#define FIRMWARE_STATUS_OFFSET			(1)
#define FIRMWARE_FILE_SIZE_OFFSET		(2)
#define FIRMWARE_CRC_OFFSET					(3)
#define FIRMWARE_START_ADDR_OFFSET	(4)
#define FIRMWARE_ERR_ADDR_OFFSET		(5)

#define SST25_OK	0
#define SST25_FAIL	1
#define CMD_READ25				0x03
#define CMD_READ80				0x0B
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_32K			0x52
#define CMD_ERASE_64K			0xD8
#define CMD_ERASE_ALL			0x60
#define CMD_WRITE_BYTE			0x02
#define CMD_WRITE_WORD_AAI		0xAD
#define CMD_READ_STATUS			0x05
#define CMD_WRITE_STATUS_ENABLE	0x50
#define CMD_WRITE_STATUS		0x01
#define CMD_WRITE_ENABLE		0x06
#define CMD_WRITE_DISABLE		0x04
#define CMD_READ_ID				0x90
#define CMD_READ_JEDEC_ID		0x9F
#define CMD_EBSY				0x70
#define CMD_DBSY				0x80

enum SST25_ERASE_MODE {
    all,
    block4k,
    block32k,
    block64k
};

uint8_t SST25_Init(void);
uint8_t SST25_Erase(uint32_t addr, enum SST25_ERASE_MODE mode);
uint8_t SST25_EraseAll(void);
uint8_t SST25_Read(uint32_t addr, uint8_t *buf, uint32_t count);
uint8_t SST25_Write(uint32_t addr, const uint8_t *buf, uint32_t count);
uint8_t _SST25_Read(uint32_t addr, uint8_t *buf, uint32_t count);
uint8_t _SST25_Write(uint32_t addr, const uint8_t *buf, uint32_t count);
void SPItest1(void);
#endif
