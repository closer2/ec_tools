#define  TOOLS_VER   "V2.0"

//*****************************************
// BatteryTool Version : 2.0
//*****************************************
// Init IT-557x chip

//*****************************************
// BatteryTool Version : 1.0
//*****************************************
// First release


/* Copyright (C)Copyright 2005-2020 ZXQ Telecom. All rights reserved.

   Author: Morgen Zhu
   
   Description:These functions of this file are reference only in the Windows!
   It can read/write ITE-EC RAM by 
   ----PM-port(62/66)
   ----KBC-port(60/64)
   ----EC-port(2E/2F or 4E/4F)
   ----Decicated I/O Port(301/302/303)
*/


// Using VS2012 X86 cmd tool to compilation
// For windows-32/64bit

//=======================================Include file ==============================================
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#include <wincon.h>
#include <Powrprof.h>
#include <Winbase.h>



using namespace std;
#include "winio.h"
//#pragma comment(lib,"WinIo.lib")       // For 32bit
#pragma comment(lib,"WinIox64.lib")    // For 64bit

#pragma comment(lib, "Powrprof.lib")
//==================================================================================================

//========================================Type Define ==============================================
typedef unsigned char   BYTE;
typedef unsigned char   UINT8;
#define TRUE            1
#define FALSE           0
//==================================================================================================

//==========================The hardware port to read/write function================================
#define READ_PORT(port,data2)  GetPortVal(port, &data2, 1);
#define WRITE_PORT(port,data2) SetPortVal(port, data2, 1)
//==================================================================================================

//======================================== PM channel ==============================================
#define PM_STATUS_PORT66          0x66
#define PM_CMD_PORT66             0x66
#define PM_DATA_PORT62            0x62
#define PM_OBF                    0x01
#define PM_IBF                    0x02
//------------wait EC PM channel port66 output buffer full-----/
void Wait_PM_OBF (void)
{
    DWORD data;
    READ_PORT(PM_STATUS_PORT66, data);
    while(!(data& PM_OBF))
    {
        READ_PORT(PM_STATUS_PORT66, data);
    }
}

//------------wait EC PM channel port66 input buffer empty-----/
void Wait_PM_IBE (void)
{
    DWORD data;
    READ_PORT(PM_STATUS_PORT66, data);
    while(data& PM_IBF)
    {
        READ_PORT(PM_STATUS_PORT66, data);
    }
}

//------------send command by EC PM channel--------------------/
void Send_cmd_by_PM(BYTE Cmd)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_CMD_PORT66, Cmd);
    Wait_PM_IBE();
}

//------------send data by EC PM channel-----------------------/
void Send_data_by_PM(BYTE Data)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_DATA_PORT62, Data);
    Wait_PM_IBE();
}

//-------------read data from EC PM channel--------------------/
BYTE Read_data_from_PM(void)
{
    DWORD data;
    Wait_PM_OBF();
    READ_PORT(PM_DATA_PORT62, data);
    return(data);
}
//--------------write EC RAM-----------------------------------/
void EC_WriteByte_PM(BYTE index, BYTE data)
{
    Send_cmd_by_PM(0x81);
    Send_data_by_PM(index);
    Send_data_by_PM(data);
}
//--------------read EC RAM------------------------------------/
BYTE EC_ReadByte_PM(BYTE index)
{
    BYTE data;
    Send_cmd_by_PM(0x80);
    Send_data_by_PM(index);
    data = Read_data_from_PM();
    return data;
}
//==================================================================================================

//================================KBC channel=======================================================
#define KBC_STATUS_PORT64         0x64
#define KBC_CMD_PORT64            0x64
#define KBC_DATA_PORT60           0x60
#define KBC_OBF                   0x01
#define KBC_IBF                   0x02
// wait EC KBC channel port64 output buffer full
void Wait_KBC_OBF (void)
{   
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(!(data& KBC_OBF))
    {
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// wait EC KBC channel port64 output buffer empty
void Wait_KBC_OBE (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(data& KBC_OBF)
    {
        READ_PORT(KBC_DATA_PORT60, data);
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// wait EC KBC channel port64 input buffer empty
void Wait_KBC_IBE (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(data& KBC_IBF)
    {
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// send command by EC KBC channel
void Send_cmd_by_KBC (BYTE Cmd)
{
    Wait_KBC_OBE();
    Wait_KBC_IBE();
    WRITE_PORT(KBC_CMD_PORT64, Cmd);
    Wait_KBC_IBE();
}

// send data by EC KBC channel
void Send_data_by_KBC (BYTE Data)
{
    Wait_KBC_OBE();
    Wait_KBC_IBE();
    WRITE_PORT(KBC_DATA_PORT60, Data);
    Wait_KBC_IBE();
}

// read data from EC KBC channel
BYTE Read_data_from_KBC(void)
{
    DWORD data;
    Wait_KBC_OBF();
    READ_PORT(KBC_DATA_PORT60, data);
    return(data);
}
// Write EC RAM via KBC port(60/64)
void EC_WriteByte_KBC(BYTE index, BYTE data)
{
    Send_cmd_by_KBC(0x81);
    Send_data_by_KBC(index);
    Send_data_by_KBC(data);
}

// Read EC RAM via KBC port(60/64)
BYTE EC_ReadByte_KBC(BYTE index)
{
    Send_cmd_by_KBC(0x80);
    Send_data_by_KBC(index);
    return Read_data_from_KBC();
}
//==================================================================================================

//=======================================EC Direct Access interface=================================
//Port Config:
//  BADRSEL(0x200A) bit1-0  Addr    Data
//                  00      2Eh     2Fh
//                  01      4Eh     4Fh
//
//              01      4Eh     4Fh
//  ITE-EC Ram Read/Write Algorithm:
//  Addr    w   0x2E
//  Data    w   0x11
//  Addr    w   0x2F
//  Data    w   high byte
//  Addr    w   0x2E
//  Data    w   0x10
//  Addr    w   0x2F
//  Data    w   low byte
//  Addr    w   0x2E
//  Data    w   0x12
//  Addr    w   0x2F
//  Data    rw  value
UINT8 EC_ADDR_PORT = 0x4E;   // 0x2E or 0x4E
UINT8 EC_DATA_PORT = 0x4F;   // 0x2F or 0x4F
UINT8 High_Byte    = 0;
// Write EC RAM via EC port(2E/2F or 4E/4F)
void ECRamWrite_Direct(unsigned short iIndex, BYTE data)
{
    DWORD data1;
    data1 = data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, High_Byte); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, data1);
}

UINT8 ECRamRead_Direct(UINT8 iIndex)
{
    DWORD data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, High_Byte); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    READ_PORT(EC_DATA_PORT, data);
    return(data);
}


unsigned char ECRamReadExt_Direct(unsigned short iIndex)
{
    DWORD data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex>>8); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex&0xFF);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    READ_PORT(EC_DATA_PORT, data);
    return(data);
}

void ECRamWriteExt_Direct(unsigned short iIndex, BYTE data)
{
    DWORD data1;
    data1 = data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex>>8);    // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex&0xFF);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, data1);
}
//==================================================================================================

//=======================================Decicated I/O Port Operation===============================
// Need EC code configuration and need BIOS decode I/O
#define HIGH_BYTE_PORT    0x301
#define LOW_BYTE_PORT     (HIGH_BYTE_PORT+1)
#define DATA_BYTE_PORT    (HIGH_BYTE_PORT+2)  // Decicated I/O Port

BYTE EC_ReadByte_DeIO(BYTE iIndex)
{
    DWORD data;
    SetPortVal(HIGH_BYTE_PORT, High_Byte, 1);
    SetPortVal(LOW_BYTE_PORT, iIndex, 1);
    GetPortVal(DATA_BYTE_PORT, &data, 1);
    return data;
}

void EC_WriteByte_DeIO(BYTE iIndex, BYTE data)
{
    SetPortVal(HIGH_BYTE_PORT, High_Byte, 1);
    SetPortVal(LOW_BYTE_PORT, iIndex, 1);
    SetPortVal(DATA_BYTE_PORT, data, 1);
}
//==================================================================================================

//===============================Console control interface==========================================
#define EFI_BLACK                 0x00
#define EFI_BLUE                  0x01
#define EFI_GREEN                 0x02
#define EFI_RED                   0x04
#define EFI_BRIGHT                0x08

#define EFI_CYAN                  (EFI_BLUE | EFI_GREEN)
#define EFI_MAGENTA               (EFI_BLUE | EFI_RED)
#define EFI_BROWN                 (EFI_GREEN | EFI_RED)
#define EFI_LIGHTGRAY             (EFI_BLUE | EFI_GREEN | EFI_RED)
#define EFI_LIGHTBLUE             (EFI_BLUE | EFI_BRIGHT)
#define EFI_LIGHTGREEN            (EFI_GREEN | EFI_BRIGHT)
#define EFI_LIGHTCYAN             (EFI_CYAN | EFI_BRIGHT)
#define EFI_LIGHTRED              (EFI_RED | EFI_BRIGHT)
#define EFI_LIGHTMAGENTA          (EFI_MAGENTA | EFI_BRIGHT)
#define EFI_YELLOW                (EFI_BROWN | EFI_BRIGHT)
#define EFI_WHITE                 (EFI_BLUE | EFI_GREEN | EFI_RED | EFI_BRIGHT)

void SetTextColor(UINT8 TextColor, UINT8 BackColor)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, (TextColor|(BackColor<<4)));
}

void SetPosition_X_Y(UINT8 PositionX, UINT8 PositionY)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos={PositionX,PositionY};
    SetConsoleCursorPosition(hOut, pos);
}

void SetToolCursor()
{
    system("cls");
    system("color 07");
    
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO CursorInfo;  
    GetConsoleCursorInfo(handle, &CursorInfo);
    CursorInfo.bVisible = false;
    SetConsoleCursorInfo(handle, &CursorInfo);
}

void ClearToolCursor()
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);  
    CONSOLE_CURSOR_INFO CursorInfo;  
    GetConsoleCursorInfo(handle, &CursorInfo);
    CursorInfo.bVisible = true;
    SetConsoleCursorInfo(handle, &CursorInfo);
    
    SetTextColor(EFI_LIGHTGRAY, EFI_BLACK);
    system("cls");
}
//==================================================================================================

//  Example:
//  ECRamWrite_Direct(0x51,0x90);
//  EC_WriteByte_KBC(0x52,0x91);
//  EC_WriteByte_PM(0x53,0x93);
//  EC_WriteByte_DeIO(0x54,0x94);
//  printf("%d\n", ECRamRead_Direct(0x51));
//  printf("%d\n", EC_ReadByte_KBC(0x52));
//  printf("%d\n", EC_ReadByte_PM(0x53));
//  printf("%d\n", EC_ReadByte_DeIO(0x54));

// 0x301
//EC_ReadByte_DeIO
//EC_WriteByte_DeIO

//0x2E
//ECRamRead_Direct
//ECRamWrite_Direct
//ECRamReadExt_Direct
//ECRamWriteExt_Direct

//60/64
//EC_ReadByte_KBC
//EC_WriteByte_KBC

//62/66
//EC_ReadByte_PM
//EC_WriteByte_PM

#define  EC_RAM_WRITE  ECRamWriteExt_Direct
#define  EC_RAM_READ   ECRamReadExt_Direct

//==================================================================================================

//=======================================Tool info==================================================
#define  TOOLS_NAME  "NPCX FanView"
#define  ITE_IC      "NPCX7962"
#define  CopyRight   "(C)Copyright 2021-2030 ZXQ Telecom."
#define  DEBUG       0
#define  ESC         0x1B
#define  KEY_UP      0x48
#define  KEY_DOWN    0x50

#define KEY_O        0x4F
#define KEY_L        0x4C
#define KEY_o        0x6f
#define KEY_l        0x6c

#define KEY_W        0x57
#define KEY_S        0x53
#define KEY_w        0x77
#define KEY_s        0x73
char Key_Value;
//==================================================================================================




//==================================================================================================
typedef unsigned char      uint8_t;
typedef signed char        int8_t;

typedef unsigned short     uint16_t;
typedef signed short       int16_t;

typedef unsigned int       uint32_t;
typedef signed int         int32_t;


uint8_t inb(uint16_t io_port)
{
	DWORD in_data;
	uint8_t return_data;
	GetPortVal(io_port, &in_data, 1);
	return_data = in_data&0xFF;
	return return_data;
}

void outb(uint8_t out_data, uint16_t io_port)
{
	SetPortVal(io_port, out_data, 1);
}



#define EC_COMMAND_PROTOCOL_3 0xda

#define EC_HOST_REQUEST_VERSION 3

/**
 * struct ec_host_request - Version 3 request from host.
 * @struct_version: Should be 3. The EC will return EC_RES_INVALID_HEADER if it
 *                  receives a header with a version it doesn't know how to
 *                  parse.
 * @checksum: Checksum of request and data; sum of all bytes including checksum
 *            should total to 0.
 * @command: Command to send (EC_CMD_...)
 * @command_version: Command version.
 * @reserved: Unused byte in current protocol version; set to 0.
 * @data_len: Length of data which follows this header.
 */
struct ec_host_request {
	uint8_t struct_version;
	uint8_t checksum;
	uint16_t command;
	uint8_t command_version;
	uint8_t reserved;
	uint16_t data_len;
};

#define EC_HOST_RESPONSE_VERSION 3

/**
 * struct ec_host_response - Version 3 response from EC.
 * @struct_version: Struct version (=3).
 * @checksum: Checksum of response and data; sum of all bytes including
 *            checksum should total to 0.
 * @result: EC's response to the command (separate from communication failure)
 * @data_len: Length of data which follows this header.
 * @reserved: Unused bytes in current protocol version; set to 0.
 */
struct ec_host_response {
	uint8_t struct_version;
	uint8_t checksum;
	uint16_t result;
	uint16_t data_len;
	uint16_t reserved;
};

/* Protocol version 3 */
#define EC_LPC_ADDR_HOST_PACKET  0x800  /* Offset of version 3 packet */
#define EC_LPC_HOST_PACKET_SIZE  0x100  /* Max size of version 3 packet */

/* I/O addresses for host command */
#define EC_LPC_ADDR_HOST_DATA  0x200
#define EC_LPC_ADDR_HOST_CMD   0x204


enum ec_status {
	EC_RES_SUCCESS = 0,
	EC_RES_INVALID_COMMAND = 1,
	EC_RES_ERROR = 2,
	EC_RES_INVALID_PARAM = 3,
	EC_RES_ACCESS_DENIED = 4,
	EC_RES_INVALID_RESPONSE = 5,
	EC_RES_INVALID_VERSION = 6,
	EC_RES_INVALID_CHECKSUM = 7,
	EC_RES_IN_PROGRESS = 8,		/* Accepted, command in progress */
	EC_RES_UNAVAILABLE = 9,		/* No response available */
	EC_RES_TIMEOUT = 10,		/* We got a timeout */
	EC_RES_OVERFLOW = 11,		/* Table / data overflow */
	EC_RES_INVALID_HEADER = 12,     /* Header contains invalid data */
	EC_RES_REQUEST_TRUNCATED = 13,  /* Didn't get the entire request */
	EC_RES_RESPONSE_TOO_BIG = 14,   /* Response was too big to handle */
	EC_RES_BUS_ERROR = 15,		/* Communications bus error */
	EC_RES_BUSY = 16,		/* Up but too busy.  Should retry */
	EC_RES_INVALID_HEADER_VERSION = 17,  /* Header version invalid */
	EC_RES_INVALID_HEADER_CRC = 18,      /* Header CRC invalid */
	EC_RES_INVALID_DATA_CRC = 19,        /* Data CRC invalid */
	EC_RES_DUP_UNAVAILABLE = 20,         /* Can't resend response */

	EC_RES_MAX = UINT16_MAX		/**< Force enum to be 16 bits */
} __packed;

/*************************************************************/

/* Get fan target RPM */
#define EC_CMD_PWM_GET_FAN_TARGET_RPM 0x0020

struct ec_params_pwm_get_fan_rpm {
	uint8_t fan_idx;
} ;

struct ec_response_pwm_get_fan_rpm {
	uint32_t rpm;
} ;


/* Set target fan RPM */
#define EC_CMD_PWM_SET_FAN_TARGET_RPM 0x0021

/* Version 1 of input params */
struct ec_params_pwm_set_fan_target_rpm_v1 {
	uint32_t rpm;
	uint8_t fan_idx;
} __ec_align_size1;

/*************************************************************/

#define EC_LPC_STATUS_FROM_HOST   0x02
#define EC_LPC_STATUS_PROCESSING  0x04

#define INITIAL_UDELAY 5     /* 5 us */
#define MAXIMUM_UDELAY 10000 /* 10 ms */
/* Don't use a macro where an inline will do... */
static inline int MIN(int a, int b) { return a < b ? a : b; }
static inline int MAX(int a, int b) { return a > b ? a : b; }
#define EC_LPC_STATUS_BUSY_MASK \
	(EC_LPC_STATUS_FROM_HOST | EC_LPC_STATUS_PROCESSING)

static int wait_for_ec(int status_addr, int timeout_usec)
{
	int i;
	int delay = INITIAL_UDELAY;

	for (i = 0; i < timeout_usec; i += delay) {
		/*
		 * Delay first, in case we just sent out a command but the EC
		 * hasn't raised the busy flag.  However, I think this doesn't
		 * happen since the LPC commands are executed in order and the
		 * busy flag is set by hardware.  Minor issue in any case,
		 * since the initial delay is very short.
		 */
		_sleep(MIN(delay, timeout_usec - i)); // millisecond

		if (!(inb(status_addr) & EC_LPC_STATUS_BUSY_MASK))
			return 0;

		/* Increase the delay interval after a few rapid checks */
		if (i > 20)
			delay = MIN(delay * 2, MAXIMUM_UDELAY);
	}
	return -1;  /* Timeout */
}


#define EECRESULT 1000
static int ec_command_lpc_3(int command, int version,
			  const void *outdata, int outsize,
			  void *indata, int insize)
{
	struct ec_host_request rq;
	struct ec_host_response rs;
	const uint8_t *d;
	uint8_t *dout;
	int csum = 0;
	int i;

	/* Fail if output size is too big */
	if (outsize + sizeof(rq) > EC_LPC_HOST_PACKET_SIZE)
		return -EC_RES_REQUEST_TRUNCATED;

	/* Fill in request packet */
	/* TODO(crosbug.com/p/23825): This should be common to all protocols */
	rq.struct_version = EC_HOST_REQUEST_VERSION;
	rq.checksum = 0;
	rq.command = command;
	rq.command_version = version;
	rq.reserved = 0;
	rq.data_len = outsize;

	/* Copy data and start checksum */
	for (i = 0, d = (const uint8_t *)outdata; i < outsize; i++, d++) {
		outb(*d, EC_LPC_ADDR_HOST_PACKET + sizeof(rq) + i);
		csum += *d;
	}

	/* Finish checksum */
	for (i = 0, d = (const uint8_t *)&rq; i < sizeof(rq); i++, d++)
		csum += *d;

	/* Write checksum field so the entire packet sums to 0 */
	rq.checksum = (uint8_t)(-csum);

	/* Copy header */
	for (i = 0, d = (const uint8_t *)&rq; i < sizeof(rq); i++, d++)
		outb(*d, EC_LPC_ADDR_HOST_PACKET + i);

	/* Start the command */
	outb(EC_COMMAND_PROTOCOL_3, EC_LPC_ADDR_HOST_CMD);

	if (wait_for_ec(EC_LPC_ADDR_HOST_CMD, 1000000)) {
		fprintf(stderr, "Timeout waiting for EC response\n");
		return -EC_RES_ERROR;
	}

	/* Check result */
	i = inb(EC_LPC_ADDR_HOST_DATA);
	if (i) {
		fprintf(stderr, "EC returned error result code %d\n", i);
		return -EECRESULT - i;
	}

	/* Read back response header and start checksum */
	csum = 0;
	for (i = 0, dout = (uint8_t *)&rs; i < sizeof(rs); i++, dout++) {
		*dout = inb(EC_LPC_ADDR_HOST_PACKET + i);
		csum += *dout;
	}

	if (rs.struct_version != EC_HOST_RESPONSE_VERSION) {
		fprintf(stderr, "EC response version mismatch\n");
		return -EC_RES_INVALID_RESPONSE;
	}

	if (rs.reserved) {
		fprintf(stderr, "EC response reserved != 0\n");
		return -EC_RES_INVALID_RESPONSE;
	}

	if (rs.data_len > insize) {
		fprintf(stderr, "EC returned too much data\n");
		fprintf(stderr, "%d--%d\n", rs.data_len, insize);
		//return -EC_RES_RESPONSE_TOO_BIG;
	}

	/* Read back data and update checksum */
	for (i = 0, dout = (uint8_t *)indata; i < rs.data_len; i++, dout++) {
		*dout = inb(EC_LPC_ADDR_HOST_PACKET + sizeof(rs) + i);
		csum += *dout;
	}

	/* Verify checksum */
	if ((uint8_t)csum) {
		fprintf(stderr, "EC response has invalid checksum\n");
		return -EC_RES_INVALID_CHECKSUM;
	}

	/* Return actual amount of data received */
	return rs.data_len;
}


void set_fan_rpm(uint8_t index, uint32_t rpm)
{
	struct ec_params_pwm_set_fan_target_rpm_v1 p_v1;
	int cmdver = 1;
	
	p_v1.fan_idx = index;
	p_v1.rpm = rpm;

	ec_command_lpc_3(EC_CMD_PWM_SET_FAN_TARGET_RPM, cmdver,
			&p_v1, sizeof(p_v1), NULL, 0);
}

uint32_t get_fan_targetrpm(uint8_t index)
{
	struct ec_params_pwm_get_fan_rpm p;
	struct ec_response_pwm_get_fan_rpm r;
	
	p.fan_idx = index;

	ec_command_lpc_3(EC_CMD_PWM_GET_FAN_TARGET_RPM, 0,
			&p, sizeof(p), &r, sizeof(r));
	
	return r.rpm;
}


//==================================================================================================


//=======================================Battery Info Type==========================================
#define  UI_BASE_X   25
#define  UI_BASE_Y   5

FILE *BAT_LogFile = NULL;
FILE *CfgFile = NULL;
unsigned int SetTime;
int BAT_LogFile_flag = 0;

typedef struct BatteryInfoStruct
{
    char InfoName[128];
    char CfgItemName[128];    // Read cfg file item name
    char InfoValue[128];      // All info converted to character
    int  InfoAddr_L;          // Information address low
    int  InfoAddr_H;          // Information address high
    int  InfoInt;             // Information Value
    char Active;              // If EC code does not support this item, disable it
    char ActiveLog;           // if enable, it will be creat log
}EC_BatteryInfo;

typedef enum InfoNameEnum
{
    EC_Version=0,

    Temp_Sensor1,
    Temp_Sensor2,
    Temp_Sensor3,
    Temp_Sensor4,
    Temp_Sensor5,
    Temp_Sensor6,
    Temp_Sensor7,
    Temp_Sensor8,
    Temp_Sensor9,
    Temp_Sensor10,
    Temp_Sensor11,
    Temp_Sensor12,
    Temp_Sensor13,
    //Temp_Battery,
    
    //BAT_Mode,
    //BAT_RMC,
    //BAT_FCC,
    //BAT_RealRSOC,
    //BAT_Current,
    //BAT_Voltage,
    
    FAN1_Current_RPM,
    FAN1_Goal_RPM,
    //FAN1_RPM_Level,
    //FAN1_Set_RPM,
    
    FAN2_Current_RPM,
    FAN2_Goal_RPM,
    //FAN2_RPM_Level,
    //FAN2_Set_RPM,
    
    INFONAMECOUNT         // count items and index it
}InfoNameEnum;

// The follow infomation address reference batteryview.cfg
EC_BatteryInfo BAT1_Info[] =
{
    {"EC_Version          :", "N/A", "N/A", 0x00,     0, 0, TRUE, FALSE},
    
    {"Temp_Sensor1        :", "N/A", "N/A", 0x20,     0, 0, TRUE, TRUE},
    {"Temp_Sensor2        :", "N/A", "N/A", 0x21,     0, 0, TRUE, TRUE},
    {"Temp_Sensor3        :", "N/A", "N/A", 0x22,     0, 0, TRUE, TRUE},
    {"Temp_Sensor4        :", "N/A", "N/A", 0x23,     0, 0, TRUE, TRUE},
    {"Temp_Sensor5        :", "N/A", "N/A", 0x24,     0, 0, TRUE, TRUE},
    {"Temp_Sensor6        :", "N/A", "N/A", 0x25,     0, 0, TRUE, TRUE},
    {"Temp_Sensor7        :", "N/A", "N/A", 0x26,     0, 0, TRUE, FALSE},
    {"Temp_Sensor8        :", "N/A", "N/A", 0x27,     0, 0, TRUE, FALSE},
    {"Temp_Sensor9        :", "N/A", "N/A", 0x28,     0, 0, TRUE, FALSE},
    {"Temp_Sensor10       :", "N/A", "N/A", 0x29,     0, 0, TRUE, FALSE},
    {"Temp_Sensor11       :", "N/A", "N/A", 0x2a,     0, 0, TRUE, FALSE},
    {"Temp_Sensor12       :", "N/A", "N/A", 0x2b,     0, 0, TRUE, FALSE},
    {"Temp_Sensor13       :", "N/A", "N/A", 0x2c,     0, 0, TRUE, FALSE},
    //{"Temp_Battery        :", "N/A", "N/A", 0x462,     0, 0, TRUE, FALSE},
    
    //{"BAT_Mode            :", "N/A", "N/A", 0x402,     0, 0, TRUE, FALSE},
    //{"BAT_RMC             :", "N/A", "N/A", 0x50A,     0, 0, TRUE, FALSE},
    //{"BAT_FCC             :", "N/A", "N/A", 0x50C,     0, 0, TRUE, FALSE},
    //{"RMC/FCC             :", "N/A", "N/A", 0,         0, 0, TRUE, FALSE},
    //{"BAT_Current         :", "N/A", "N/A", 0x506,     0, 0, TRUE, FALSE},
    //{"BAT_Voltage         :", "N/A", "N/A", 0x504,     0, 0, TRUE, FALSE},
    
    {"FAN1_Current_RPM    :", "N/A", "N/A", 0x57,   0x56, 0, TRUE, TRUE},
    {"FAN1_Goal_RPM       :", "N/A", "N/A", 0x462,     0, 0, TRUE, TRUE},
    //{"FAN1_RPM_Level      :", "N/A", "N/A", 0x462,     0, 0, TRUE, FALSE},
    //{"FAN1_Set_RPM        :", "N/A", "N/A", 0x462,     0, 0, TRUE, FALSE},
    
    {"FAN2_Current_RPM    :", "N/A", "N/A", 0x59,   0x58, 0, TRUE, TRUE},
    {"FAN2_Goal_RPM       :", "N/A", "N/A", 0x462,     0, 0, TRUE, TRUE},
    //{"FAN2_RPM_Level      :", "N/A", "N/A", 0x462,     0, 0, TRUE, FALSE},
    //{"FAN2_Set_RPM        :", "N/A", "N/A", 0x462,     0, 0, TRUE, FALSE},
    
    {0, 0, 0, 0, 0, 0}   // end
};
//==================================================================================================

void ReadCfgFile(void)
{
    char StrLine[1024];
    char StrNum[16];
    int  InfoIndex=0;
    int  HexNum;
    char *str;
    char *pStrLine;
    int i=0;
    int j=0;
    
    if((CfgFile = fopen("FanView.cfg","r")) == NULL)
    {
        printf("FanView.cfg not exist\n");
        return ;
    }
    
    while (!feof(CfgFile))
    {   
        // Read one line data
        fgets(StrLine,1024,CfgFile);
        //printf("%s", StrLine);
        
        pStrLine = StrLine;
        if(('$'==StrLine[0]) && (('1'==StrLine[1])))
        {
            //InfoIndex = (StrLine[3]-'0')*10 + (StrLine[4]-'0');
            
            
            while(('#' != (*pStrLine++)));
            HexNum = (int)strtol(pStrLine, &str, 16);
            //printf("%#X ",HexNum);
            BAT1_Info[InfoIndex].InfoAddr_L = HexNum;
            
            pStrLine++;
            while(('#' != (*pStrLine++)));
            
            HexNum = (int)strtol(pStrLine, &str, 16);
            //printf("%#X ",HexNum);
            BAT1_Info[InfoIndex].InfoAddr_H = HexNum;
            
            pStrLine++;
            while(('#' != (*pStrLine++)));
            HexNum = (int)strtol(pStrLine, &str, 10);
            //printf("%#X\n",HexNum);
            if(0x00==HexNum)
            {
                BAT1_Info[InfoIndex].Active = 0;
                BAT1_Info[InfoIndex].ActiveLog = 0;
            }
            else if(0x01==HexNum)
            {
                BAT1_Info[InfoIndex].Active = 1;
                BAT1_Info[InfoIndex].ActiveLog = 0;
            }
            else if(0x03==HexNum)
            {
                BAT1_Info[InfoIndex].Active = 1;
                BAT1_Info[InfoIndex].ActiveLog = 1;
            }
            
            pStrLine++;
            while(('#' != (*pStrLine++)));
            j=0;
            while(('#' != (*pStrLine)))
            {
                BAT1_Info[InfoIndex].CfgItemName[j] = *pStrLine;
                j++;
                pStrLine++;
            }
            
            InfoIndex++;
        }
        else if(('$'==StrLine[0]) && (('0'==StrLine[1])))
        {
            if('0' == StrLine[3])
            {
                while(('#' != (*pStrLine++)));
                HexNum = (int)strtol(pStrLine, &str, 10);
                //printf("Log file : %d\n",HexNum);
                BAT_LogFile_flag = HexNum;
            }
            if('1' == StrLine[3])
            {
                while(('#' != (*pStrLine++)));
                HexNum = (int)strtol(pStrLine, &str, 16);
                //printf("IO : %#X\n",HexNum);
                EC_ADDR_PORT = HexNum;
                EC_DATA_PORT = HexNum+1;
            }
            if('2' == StrLine[3])
            {
                while(('#' != (*pStrLine++)));
                HexNum = (int)strtol(pStrLine, &str, 10);
                //printf("Time : %d\n",HexNum);
                SetTime = HexNum;
            }
        }
    }
    #if 0
    printf("\n\n\n");
    for(i=0; i<INFONAMECOUNT; i++)
    {
        printf("%d %#X %#X %#X %#X %s\n", i, BAT1_Info[i].InfoAddr_L, BAT1_Info[i].InfoAddr_H, BAT1_Info[i].Active,
                                    BAT1_Info[i].ActiveLog, BAT1_Info[i].CfgItemName);
    }
    #endif
    
    fclose(CfgFile);
}

void ToolInit(void)
{
    int i,j;
    /*unsigned char EC_CHIP_ID1;
    unsigned char EC_CHIP_ID2;
    unsigned char EC_CHIP_Ver;
    
    // ITE IT-557x chip is DLM architecture for EC  RAM and It's support 6K/8K RAM.
    // If used RAM less  than 4K, you can access EC RAM form 0x000--0xFFF by 4E/4F IO port
    // If used RAM more than 4K, RAM address change to 0xC000
    // If you want to access EC RAM by 4E/4F IO port, you must set as follow register first
    // REG_1060[BIT7]
    EC_CHIP_ID1 = EC_RAM_READ(0x2000);
    EC_CHIP_ID2 = EC_RAM_READ(0x2001);
    if(0x55==EC_CHIP_ID1)
    {
        EC_CHIP_Ver = EC_RAM_READ(0x1060);
        EC_CHIP_Ver = EC_CHIP_Ver | 0x80;
        EC_RAM_WRITE(0x1060, EC_CHIP_Ver);
    }
    
    SetConsoleTitle(TOOLS_NAME);
    system("mode con cols=95 lines=60");*/
    
    printf("Fan Tool %s (For ITE %s EC code)\n",TOOLS_VER, ITE_IC);
    printf("%s All rights reserved.\n",CopyRight);
    
    SetTextColor(EFI_YELLOW, EFI_BLACK);
    printf("O/L For Fan1 Speed Control, W/S For Fan2 Speed Control\n");
    SetTextColor(EFI_LIGHTMAGENTA, EFI_BLACK);
    printf("<ESC> to exit!");
    SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
    for(i=0,j=0;i<INFONAMECOUNT;i++)
    {
        if(BAT1_Info[i].Active)
        {
            SetPosition_X_Y(0, UI_BASE_Y+j);
            printf(BAT1_Info[i].CfgItemName);
            j++;
        }
    }
    
    if(BAT1_Info[EC_Version].Active)
    {
        sprintf(BAT1_Info[EC_Version].InfoValue, "%02d.%02d.%02d",
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_L),
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H),
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H+1)
                /*EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H+2)*/);  // The EC version is 4Byte
    }
}


void PrintLogFile(void)
{
    char tmp[64];
    int i;
    
    time_t t = time(0);
    strftime( tmp, sizeof(tmp), "%Y/%m/%d/%X",localtime(&t) );
    fprintf(BAT_LogFile, "%-22s" ,tmp);

    for(i=0;i<INFONAMECOUNT;i++)
    {
        if(BAT1_Info[i].ActiveLog)
        {
            fprintf(BAT_LogFile, "%-20d", BAT1_Info[i].InfoInt);
        }
    }
    fprintf(BAT_LogFile, "\n");
    fflush(BAT_LogFile);
}

void PrintFanInfo(void)
{
    int i,j;
    for(i=0,j=0;i<INFONAMECOUNT;i++)
    {
        if(BAT1_Info[i].Active)
        {
            SetPosition_X_Y(UI_BASE_X, UI_BASE_Y+j);
            SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
            printf("%s", BAT1_Info[i].InfoValue);
            j++;
        }
    }
}

void PollFanInfo(void)
{
    unsigned int tmpvalue;
    float tmpvalue1;
	uint32_t targetrpm;
    
    /*if(BAT1_Info[Temp_Battery].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Battery].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Temp_Battery].InfoAddr_L);
        tmpvalue1 = tmpvalue*0.1-273.15;
        BAT1_Info[Temp_Battery].InfoInt = tmpvalue1;
        sprintf(BAT1_Info[Temp_Battery].InfoValue, "%-8.1f C", tmpvalue1);
    }*/
    if(BAT1_Info[Temp_Sensor1].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor1].InfoAddr_L);
        BAT1_Info[Temp_Sensor1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor1].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor1].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor2].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor2].InfoAddr_L);
        BAT1_Info[Temp_Sensor2].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor2].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor2].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor3].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor3].InfoAddr_L);
        BAT1_Info[Temp_Sensor3].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor3].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor3].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor4].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor4].InfoAddr_L);
        BAT1_Info[Temp_Sensor4].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor4].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor4].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor5].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor5].InfoAddr_L);
        BAT1_Info[Temp_Sensor5].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor5].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor5].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor6].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor6].InfoAddr_L);
        BAT1_Info[Temp_Sensor6].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor6].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor6].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor7].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor7].InfoAddr_L);
        BAT1_Info[Temp_Sensor7].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor7].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor7].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor8].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor8].InfoAddr_L);
        BAT1_Info[Temp_Sensor8].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor8].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor8].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor9].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor9].InfoAddr_L);
        BAT1_Info[Temp_Sensor9].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor9].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor9].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor10].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor10].InfoAddr_L);
        BAT1_Info[Temp_Sensor10].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor10].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor10].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor11].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor11].InfoAddr_L);
        BAT1_Info[Temp_Sensor11].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor11].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor11].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor12].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor12].InfoAddr_L);
        BAT1_Info[Temp_Sensor12].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor12].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor12].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor13].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[Temp_Sensor13].InfoAddr_L);
        BAT1_Info[Temp_Sensor13].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor13].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor13].InfoInt);
    }
	
    /*
    if(BAT1_Info[BAT_Current].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Current].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Current].InfoAddr_L);

        if(tmpvalue>0x8000)
        {
            tmpvalue ^=0xFFFF;
            tmpvalue+=1;
            tmpvalue = -tmpvalue;
        }

        BAT1_Info[BAT_Current].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Current].InfoValue, "%-8d mA",
                BAT1_Info[BAT_Current].InfoInt);
    }
    if(BAT1_Info[BAT_Voltage].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Voltage].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Voltage].InfoAddr_L);
        BAT1_Info[BAT_Voltage].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Voltage].InfoValue, "%-8d mV",
                BAT1_Info[BAT_Voltage].InfoInt);
    }
    if(BAT1_Info[BAT_Mode].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Mode].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Mode].InfoAddr_L);
        BAT1_Info[BAT_Mode].InfoInt = tmpvalue;
        
        sprintf(BAT1_Info[BAT_Mode].InfoValue, "%04X [%-5s]",
                BAT1_Info[BAT_Mode].InfoInt, ((tmpvalue&0x8000)?"mWh":"mAh")); // mAh or 10mWh
    }
    if(BAT1_Info[BAT_RMC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_RMC].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_RMC].InfoAddr_L);
        if(BAT1_Info[BAT_Mode].InfoInt&0x8000)
        {
            tmpvalue = tmpvalue*10;
        }
        BAT1_Info[BAT_RMC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_RMC].InfoValue, "%-8d %s",
                BAT1_Info[BAT_RMC].InfoInt,
                ((BAT1_Info[BAT_Mode].InfoInt&0x8000)?"mWh":"mAh"));
    }
    if(BAT1_Info[BAT_FCC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_FCC].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_FCC].InfoAddr_L);
        if(BAT1_Info[BAT_Mode].InfoInt&0x8000)
        {
            tmpvalue = tmpvalue*10;
        }
        BAT1_Info[BAT_FCC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_FCC].InfoValue, "%-8d %s",
                BAT1_Info[BAT_FCC].InfoInt,
                ((BAT1_Info[BAT_Mode].InfoInt&0x8000)?"mWh":"mAh"));
    }
    if(BAT1_Info[BAT_RealRSOC].Active)
    {
        if(BAT1_Info[BAT_FCC].InfoInt)
        {
            tmpvalue1 = (BAT1_Info[BAT_RMC].InfoInt*1.0 / BAT1_Info[BAT_FCC].InfoInt)*100;
        }
        else
        {
            tmpvalue1=0;
        }
        sprintf(BAT1_Info[BAT_RealRSOC].InfoValue, "%-8.2f %%", tmpvalue1);
        BAT1_Info[BAT_RealRSOC].InfoInt = (int)(tmpvalue1+0.5);
    }
	*/
    
    if(BAT1_Info[FAN1_Current_RPM].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[FAN1_Current_RPM].InfoAddr_H)<<8
                | EC_ReadByte_PM(BAT1_Info[FAN1_Current_RPM].InfoAddr_L);
        BAT1_Info[FAN1_Current_RPM].InfoInt = tmpvalue;
        sprintf(BAT1_Info[FAN1_Current_RPM].InfoValue, "%-8d ",BAT1_Info[FAN1_Current_RPM].InfoInt);
    }
    if(BAT1_Info[FAN1_Goal_RPM].Active)
    {
		targetrpm = get_fan_targetrpm(0);
		BAT1_Info[FAN1_Goal_RPM].InfoInt = targetrpm;
		sprintf(BAT1_Info[FAN1_Goal_RPM].InfoValue, "%-8d ",BAT1_Info[FAN1_Goal_RPM].InfoInt);
    }/*
    if(BAT1_Info[FAN1_RPM_Level].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[FAN1_RPM_Level].InfoAddr_L);
        BAT1_Info[FAN1_RPM_Level].InfoInt = tmpvalue;
        sprintf(BAT1_Info[FAN1_RPM_Level].InfoValue, "%-8d ",BAT1_Info[FAN1_RPM_Level].InfoInt);
    }*/
    
    if(BAT1_Info[FAN2_Current_RPM].Active)
    {
        tmpvalue = EC_ReadByte_PM(BAT1_Info[FAN2_Current_RPM].InfoAddr_H)<<8
                | EC_ReadByte_PM(BAT1_Info[FAN2_Current_RPM].InfoAddr_L);
        BAT1_Info[FAN2_Current_RPM].InfoInt = tmpvalue;
        sprintf(BAT1_Info[FAN2_Current_RPM].InfoValue, "%-8d ",BAT1_Info[FAN2_Current_RPM].InfoInt);
    }
    if(BAT1_Info[FAN2_Goal_RPM].Active)
    {
        targetrpm = get_fan_targetrpm(1);
        BAT1_Info[FAN2_Goal_RPM].InfoInt = targetrpm;
        sprintf(BAT1_Info[FAN2_Goal_RPM].InfoValue, "%-8d ",BAT1_Info[FAN2_Goal_RPM].InfoInt);
    }/*
    if(BAT1_Info[FAN2_RPM_Level].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[FAN2_RPM_Level].InfoAddr_L);
        BAT1_Info[FAN2_RPM_Level].InfoInt = tmpvalue;
        sprintf(BAT1_Info[FAN2_RPM_Level].InfoValue, "%-8d ",BAT1_Info[FAN2_RPM_Level].InfoInt);
    }*/
}

void Key_Manage()
{
	int rpm1 = BAT1_Info[FAN1_Goal_RPM].InfoInt;
	int rpm2 = BAT1_Info[FAN2_Goal_RPM].InfoInt;
	
    if(KEY_O==Key_Value || KEY_o==Key_Value || KEY_UP==Key_Value)
    {
		if(rpm1 < 2800)
        {
			set_fan_rpm(0, rpm1+100);
        }
    }
    else if(KEY_L==Key_Value || KEY_l==Key_Value || KEY_DOWN==Key_Value)
    {
    	if(rpm1 > 220)
        {
			set_fan_rpm(0, rpm1-100);
        }
    }
    else if((KEY_W==Key_Value) || (KEY_w==Key_Value))
    {
    	if(rpm2 < 2800)
        {
			set_fan_rpm(1, rpm2+100);
        }
    }
    else if((KEY_S==Key_Value) || (KEY_s==Key_Value))
    {
    	if(rpm2 > 220)
        {
			set_fan_rpm(1, rpm2-100);
        }
    }
}

void display(void)
{
    char i;
    //SetPosition_X_Y(UI_BASE_X, UI_BASE_Y+37);
    printf("\n                                          \r");
    for(i=0;i<10;i++)
    {
        printf("%c",'#');
        //Sleep(SetTime);
        _sleep(SetTime);   // millisecond
        if(kbhit())
        {
            Key_Value=getch();
            Key_Manage();
        }
    }
}

int main(int argc, char *argv[])
{
    char IOInitOK=0;
    int i;

    IOInitOK = InitializeWinIo();
    if(IOInitOK)
    {
        SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
        printf("WinIo OK\n");
    }
    else
    {
        SetTextColor(EFI_LIGHTRED, EFI_BLACK);
        printf("Error during initialization of WinIo\n");
        goto IOError;
    }
    
    SetTextColor(EFI_WHITE, EFI_BLACK);
    system("cls");
    
    // Read battery.cfg file to init battery info address
    ReadCfgFile();
    
    if(NULL == CfgFile)
    {
        goto end;
    }
    
    ToolInit();
    PollFanInfo();
    
    //---------------------------------------Creat log file---------------------------------------------
    if(BAT_LogFile_flag)
    {
        time_t t = time(0);
        char tmp[64];
        strftime( tmp, sizeof(tmp), "%Y-%m-%d[%X]",localtime(&t) );
        tmp[13] = '.';
        tmp[16] = '.';
        strcat(tmp,"log.txt");
        BAT_LogFile = fopen(tmp,"w");
        
        fprintf(BAT_LogFile, "Fan Tool %s (For ITE %s EC code)\n",TOOLS_VER, ITE_IC);
        fprintf(BAT_LogFile, "EC current version is : %s\n", BAT1_Info[EC_Version].InfoValue);
        fprintf(BAT_LogFile, "Set the data polling time  is %d(s)\n" ,SetTime/100);
        
        fprintf(BAT_LogFile, "%-22s", "Date&Time");
        
        for(i=0;i<INFONAMECOUNT;i++)
        {
            if(BAT1_Info[i].ActiveLog)
            {
                fprintf(BAT_LogFile, "%-20.18s", BAT1_Info[i].CfgItemName);
            }
        }
        fprintf(BAT_LogFile, "\n");
        fflush(BAT_LogFile);
    }
//--------------------------------------------------------------------------------------------------
    
    while(ESC!=Key_Value)
    {
        PollFanInfo();
        
        if(BAT_LogFile)
        {
            PrintLogFile();
        }
        
        PrintFanInfo();
        
        display();
    }
    
    if(BAT_LogFile)
    {
        fclose(BAT_LogFile);
    }
    /*
    //--------------------------------------------
    // Clear Fan test falg
    EC_RAM_WRITE(BAT1_Info[FAN1_Set_RPM].InfoAddr_L,0);
    EC_RAM_WRITE(BAT1_Info[FAN2_Set_RPM].InfoAddr_L,0);
    EC_RAM_WRITE(0xB20,0);*/
    //--------------------------------------------

    goto end;

IOError:
    ShutdownWinIo();
    return 1;
    
end:
ShutdownWinIo();
    return 0;
}
