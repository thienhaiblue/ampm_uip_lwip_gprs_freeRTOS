#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "uart3.h"
///* The ITM port is used to direct the printf() output to the serial window in 
//the Keil simulator IDE. */
//#define mainITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
//#define mainITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
//#define mainDEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
//#define mainTRCENA          0x01000000


//int fputc( int iChar, FILE *pxNotUsed ) 
//{
//	/* Just to avoid compiler warnings. */
//	( void ) pxNotUsed;

//	if( mainDEMCR & mainTRCENA ) 
//	{
//		while( mainITM_Port32( 0 ) == 0 );
//		mainITM_Port8( 0 ) = iChar;
//  	}

//  	return( iChar );
//}

uint32_t  DbgCfgPrintf(const uint8_t *format, ...)
{
	static  uint8_t  buffer[256];
	uint32_t len,i;
	__va_list     vArgs;		    
	va_start(vArgs, format);
	len = vsprintf((char *)&buffer[0], (char const *)format, vArgs);
	va_end(vArgs);
	if(len >= 255) len = 255;
	for(i = 0;i < len; i++)
	{
			USART3_PutChar(buffer[i]);
	}
	return 0;
}

