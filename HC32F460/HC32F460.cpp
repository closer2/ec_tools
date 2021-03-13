#define  TOOLS_VER   "V0.1"

/* Copyright (C)Copyright 2020 Bitland Telecom. All rights reserved.

   Author: MorgenZhu,ZhouHao
   
   Description: These functions of this file are reference only in the Windows!
   It can read/write chromium-EC RAM by 
   ----PM-port(62/66)
   ----Decicated I/O(800) I/O(900)
   
    Using VS2015 X86 cmd tool to compilation For windows-32/64bit
*/
// For windows-32/64bit

//=======================================Include file ==========================
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
#include <time.h>




using namespace std;
#include "winio.h"
//#pragma comment(lib,"WinIo.lib")       // For 32bit
#pragma comment(lib,"WinIox64.lib")    // For 64bit

#pragma comment(lib, "Powrprof.lib")
//==============================================================================

//============================Data Type Define =================================
typedef unsigned char      uint8_t;
typedef signed char        int8_t;

typedef unsigned short     uint16_t;
typedef signed short       int16_t;

typedef unsigned int       uint32_t;
typedef signed int         int32_t;
#define TRUE            1
#define FALSE           0
#define TOOL_DEBUG		1

#ifndef BIT
#define BIT(nr)         (1UL << (nr))
#endif
//==============================================================================


//==========================The hardware port to read/write function============
#define READ_PORT(port,data2)  GetPortVal(port, &data2, 1);
#define WRITE_PORT(port,data2) SetPortVal(port, data2, 1)
//==============================================================================

//======================================== PM channel ==========================
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
//==============================================================================

//===============================Console control interface======================
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
//==============================================================================

//  Example:
//  EC_WriteByte_PM(0x53,0x93);
//  printf("%d\n", ECRamRead_Direct(0x51));
//  printf("%d\n", EC_ReadByte_KBC(0x52));
//  printf("%d\n", EC_ReadByte_PM(0x53));
//  printf("%d\n", EC_ReadByte_DeIO(0x54));

//62/66
//EC_ReadByte_PM
//EC_WriteByte_PM

#define  EC_RAM_WRITE  EC_WriteByte_PM
#define  EC_RAM_READ   EC_ReadByte_PM

//==============================================================================

//=======================================Tool info==============================
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
//==============================================================================




//==============================================================================


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

/******************************************************************************/
/* I2C passthru command */

#define EC_CMD_I2C_PASSTHRU 0x009E

/* Read data; if not present, message is a write */
#define EC_I2C_FLAG_READ	BIT(15)

/* Mask for address */
#define EC_I2C_ADDR_MASK	0x3ff

#define EC_I2C_STATUS_NAK	BIT(0) /* Transfer was not acknowledged */
#define EC_I2C_STATUS_TIMEOUT	BIT(1) /* Timeout during transfer */

/* Any error */
#define EC_I2C_STATUS_ERROR	(EC_I2C_STATUS_NAK | EC_I2C_STATUS_TIMEOUT)

struct ec_params_i2c_passthru_msg {
    uint16_t addr_flags;	/* I2C slave address and flags */
    uint16_t len;		/* Number of bytes to read or write */
};

struct ec_params_i2c_passthru {
    uint8_t port;		/* I2C port number */
    uint8_t num_msgs;	/* Number of messages */
    struct ec_params_i2c_passthru_msg msg[];
    /* Data to write for all messages is concatenated here */
};

struct ec_response_i2c_passthru {
    uint8_t i2c_status;	/* Status flags (EC_I2C_STATUS_...) */
    uint8_t num_msgs;	/* Number of messages processed */
    uint8_t data[];		/* Data read by messages concatenated here */
};

/******************************************************************************/
#define EC_LPC_STATUS_FROM_HOST   0x02
#define EC_LPC_STATUS_PROCESSING  0x04

#define INITIAL_UDELAY 5     /* 5 us */
#define MAXIMUM_UDELAY 10000 /* 10 ms */
/* Don't use a macro where an inline will do... */
static inline int MIN(int a, int b) { return a < b ? a : b; }
static inline int MAX(int a, int b) { return a > b ? a : b; }
#define EC_LPC_STATUS_BUSY_MASK \
    (EC_LPC_STATUS_FROM_HOST | EC_LPC_STATUS_PROCESSING)

int ec_max_outsize, ec_max_insize;
void *ec_outbuf;
void *ec_inbuf;


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

int comm_init_lpc(void)
{
	/*
	* Test if LPC command args are supported.
	*
	* The cheapest way to do this is by looking for the memory-mapped
	* flag.  This is faster than sending a new-style 'hello' command and
	* seeing whether the EC sets the EC_HOST_ARGS_FLAG_FROM_HOST flag
	* in args when it responds.
	*/

	/*
	* We have modified the EC ACPI RAM definition.
	* Therefore, the "EC" character cannot be checked.
	*  Morgen@2021/01/02
	* */
#if 0
	if (inb(EC_LPC_ADDR_MEMMAP + EC_MEMMAP_ID) != 'E' ||
	    inb(EC_LPC_ADDR_MEMMAP + EC_MEMMAP_ID + 1) != 'C') {
	    fprintf(stderr, "Missing Chromium EC memory map.\n");
	    return -5;
	}
#endif

	ec_max_outsize = EC_LPC_HOST_PACKET_SIZE -
		sizeof(struct ec_host_request);
	ec_max_insize = EC_LPC_HOST_PACKET_SIZE -
		sizeof(struct ec_host_response);

	ec_outbuf = malloc(ec_max_outsize);
	ec_inbuf = malloc(ec_max_insize);

	return 0;
}

#if 0
uint32_t get_fan_targetrpm(uint8_t index)
{
    struct ec_params_pwm_get_fan_rpm p;
    struct ec_response_pwm_get_fan_rpm r;
    
    p.fan_idx = index;

    ec_command_lpc_3(EC_CMD_PWM_GET_FAN_TARGET_RPM, 0,
            &p, sizeof(p), &r, sizeof(r));
    
    return r.rpm;
}
#endif

int do_i2c_xfer(unsigned int port, unsigned int addr,
        uint8_t *write_buf, int write_len,
        uint8_t **read_buf, int read_len) {
    struct ec_params_i2c_passthru *p =
        (struct ec_params_i2c_passthru *)ec_outbuf;
    struct ec_response_i2c_passthru *r =
        (struct ec_response_i2c_passthru *)ec_inbuf;
    struct ec_params_i2c_passthru_msg *msg = p->msg;
    uint8_t *pdata;
    int size;
    int rv;

    p->port = port;
    p->num_msgs = (read_len != 0) + (write_len != 0);

    size = sizeof(*p) + p->num_msgs * sizeof(*msg);
    if (size + write_len > ec_max_outsize) {
        fprintf(stderr, "Params too large for buffer\n");
        return -1;
    }
    if (sizeof(*r) + read_len > ec_max_insize) {
        fprintf(stderr, "Read length too big for buffer\n");
        return -1;
    }

    pdata = (uint8_t *)p + size;
    if (write_len) {
        msg->addr_flags = addr;
        msg->len = write_len;

        memcpy(pdata, write_buf, write_len);
        msg++;
    }

    if (read_len) {
        msg->addr_flags = addr | EC_I2C_FLAG_READ;
        msg->len = read_len;
    }

    rv = ec_command_lpc_3(EC_CMD_I2C_PASSTHRU, 0, p, size + write_len,
            r, sizeof(*r) + read_len);
    if (rv < 0)
        return rv;

    /* Parse response */
    if (r->i2c_status & (EC_I2C_STATUS_NAK | EC_I2C_STATUS_TIMEOUT)) {
        fprintf(stderr, "Transfer failed with status=0x%x\n",
            r->i2c_status);
        return -1;
    }

    if (rv < sizeof(*r) + read_len) {
        fprintf(stderr, "Truncated read response\n");
        return -1;
    }

    if (read_len)
        *read_buf = r->data;

    return 0;
}
/******************************************************************************/

#define HC32F460_I2C_PORT           0       /* Reference board.c config */

#define HC32F460_I2C_ADDR           0x5A    /* Reference Upgrade Protocol */
#define HC32F460_REG_VER_0x01       0x01
#define HC32F460_REG_STATUS_0x02    0x02
#define HC32F460_REG_FLASHADDR_0x03 0x03

#define CMD_ID_ReadVer              0x11
#define CMD_ID_ReadSysSts           0x12
#define CMD_ID_UpdateStart          0x21
#define CMD_ID_UpdateContinue       0x22
#define CMD_ID_UpdateEnd            0x23

#define ONCES_READ_SIZE             112

FILE *binaryFile = NULL;
static uint32_t binSize = 0;
static uint32_t dataSize = 0;
static uint32_t startAddress = 0;

static uint8_t mainVersion = 0x00;
static uint8_t subVersion = 0x00;
static uint8_t hc32f460Status = 0xff;

typedef struct _IIC_HEAD{
	uint8_t seq;
	uint8_t tag;
	uint16_t lenth;
}IIC_HEAD;

/*
typedef struct _IIC_UPGRADE_PACKAGE{ 
	uint32_t startAddress;
	uint32_t dataSize;
	uint8_t dataBlob[dataSize];
}IIC_UPGRADE_PACKAGE;
*/

typedef struct _DATA_PACKET_STR{
	IIC_HEAD head;
	uint8_t data[];
}DATA_PACKET_STR;


uint8_t SumCrc(uint8_t *data, uint16_t lenth)
{
    uint8_t sum = 0;
    uint16_t i;
    
    for(i = 0; i < lenth; i++)
    {
        sum += data[i];
    }
    
    return sum;    
}

uint8_t get_firmware_size(char *filename)
{
	if((binaryFile = fopen(filename, "r")) == NULL)
	{
		printf("%s not exist\n\n", filename);
		return -1;
	}

	fseek(binaryFile, 0, SEEK_END);
	binSize = ftell(binaryFile);
	printf("binSize = %ld Byte\n", binSize);

	return 0;
}

void load_firmware_to_buf(uint8_t *data)
{
	if(binSize - startAddress < ONCES_READ_SIZE)
	{
		dataSize = binSize - startAddress;
	}
	else
	{
		dataSize = ONCES_READ_SIZE;
	}
	
	fseek(binaryFile, startAddress, SEEK_SET);
	fread(data, 1, dataSize, binaryFile);
	startAddress+=dataSize;
}

uint16_t Upgrade_Data_Code(uint8_t cmd, uint8_t *buffer)
{
    static uint8_t prev_seq = 0;
    uint16_t i = 0;
    uint16_t lenth = 0;
    DATA_PACKET_STR *strData = (DATA_PACKET_STR*)buffer;
    
    switch(cmd)
    {
    case CMD_ID_ReadVer:
    case CMD_ID_ReadSysSts:
		strData->head.seq = 0;
		strData->head.tag = cmd;
		strData->head.lenth = 0;
		lenth = sizeof(IIC_HEAD) + i;
		strData->data[i++] = SumCrc(buffer, lenth);
		lenth++;
		break;
      
    case CMD_ID_UpdateStart:
		strData->head.seq = 0;
		strData->head.tag = cmd;
		strData->head.lenth = 0;
		strData->data[i++] = 0;
		strData->data[i++] = 0;
		strData->data[i++] = 0;
		strData->data[i++] = 0;
		lenth = sizeof(IIC_HEAD) + i;
		strData->head.lenth = lenth;
		strData->data[i++] = SumCrc(buffer, lenth);
		lenth++;
		break;
      
    case CMD_ID_UpdateContinue:
		if(startAddress <= binSize)
		{
			strData->head.seq = prev_seq++;
			strData->head.tag = cmd;
			strData->head.lenth = 0;
			strData->data[i++] = startAddress;
			strData->data[i++] = (startAddress>>8);
			strData->data[i++] = (startAddress>>16);
			strData->data[i++] = (startAddress>>24);

			load_firmware_to_buf(&strData->data[i+4]);

			strData->data[i++] = dataSize;
			strData->data[i++] = (dataSize>>8);
			strData->data[i++] = (dataSize>>16);
			strData->data[i++] = (dataSize>>24);

			i += dataSize;

			lenth = sizeof(IIC_HEAD) + i;
			strData->head.lenth = lenth;
			strData->data[i++] = SumCrc(buffer, lenth);
			lenth++;
		}
		break;
      
    case CMD_ID_UpdateEnd:
		strData->head.seq = 0;
		strData->head.tag = cmd;
		strData->head.lenth = 0;
		strData->data[i++] = 1;
		lenth = sizeof(IIC_HEAD) + i;
		strData->head.lenth = lenth;
		strData->data[i++] = SumCrc(buffer, lenth);
		lenth++;
		break;
    
    default:
      break;
    }
    
    return lenth;
}


int hc32f460_i2c_read(uint8_t reg, uint8_t **read_buf, int read_len)
{
    int rv;

    if(read_len>128)
    {
        printf("read length too long\n");
        return -1;
    }

    rv = do_i2c_xfer(HC32F460_I2C_PORT, HC32F460_I2C_ADDR,
                        &reg, 1, read_buf, read_len);

    if (rv < 0)
        return rv;

    return 0;
}

int hc32f460_i2c_write(uint8_t reg, uint8_t *write_data, int write_len)
{
    uint8_t write_buf[256];
    int rv;

    if(write_len>128)
    {
        printf("write length too long\n");
        return -1;
    }

    write_buf[0] = reg;

    for(rv=0; rv<write_len; rv++)
    {
        write_buf[rv+1] = *(write_data+rv);
    }
    
    rv = do_i2c_xfer(HC32F460_I2C_PORT, HC32F460_I2C_ADDR,
                        write_buf, write_len+1, NULL, 0);

    if (rv < 0)
        return rv;

    return 0;
}

void read_hc32f460_version(void)
{
    int rv;
    uint8_t *read_buf;
    uint8_t offset;

    rv = hc32f460_i2c_read(HC32F460_REG_VER_0x01, &read_buf, (4+64+1));

    if (0 == rv)
    {
		mainVersion = read_buf[8];
		subVersion = read_buf[9];
    }
    else
        printf("HC32F460_Ver = error\n");
}

void read_hc32f460_status(void)
{
    int rv;
    uint8_t *read_buf;
    uint8_t offset;

    rv = hc32f460_i2c_read(HC32F460_REG_STATUS_0x02, &read_buf, (4+1+1));

    if (0 == rv)
    {
    	hc32f460Status = read_buf[4];
    }
    else
        printf("HC32F460_Sta = error\n");
}

int hc32f460_update_start(void)
{
	uint8_t write_buf[128];
	int write_len;
	int rv;

	printf("Package data...\n");
	write_len = Upgrade_Data_Code(CMD_ID_UpdateStart, write_buf);
	printf("Package Done\nwrite_len = %d\n", write_len);
	
	rv = hc32f460_i2c_write(HC32F460_REG_FLASHADDR_0x03, write_buf, write_len);
	if(rv < 0)
	{
		printf("CMD_ID_UpdateStart = failed!\n\n");
		return rv;
	}
	else
	{
		printf("CMD_ID_UpdateStart = success!\n\n");
		return 0;
	}
}

int hc32f460_update_end(void)
{
	uint8_t write_buf[128];
	int write_len;
	int rv;

	printf("Package data...\n");
	write_len = Upgrade_Data_Code(CMD_ID_UpdateEnd, write_buf);
	printf("Package Done\nwrite_len = %d\n", write_len);

	rv = hc32f460_i2c_write(HC32F460_REG_FLASHADDR_0x03, write_buf, write_len);
	if(rv < 0)
	{
		printf("CMD_ID_UpdateEnd = failed!\n\n");
		return rv;
	}
	else
	{
		printf("CMD_ID_UpdateEnd = success!\n\n");
		return 0;
	}
}

int hc32f460_update_continue(void)
{
	uint8_t write_buf[128];
	int write_len;
	int rv;
	
	while(startAddress < binSize)
	{
		read_hc32f460_status();
		printf("--- hc32f460Status = %02x\n", hc32f460Status);
		
		if(0x30 == hc32f460Status)
		{
			_sleep(20);
			continue;
		}
		else if(0x01 == hc32f460Status || 0x02 == hc32f460Status 
			 || 0x03 == hc32f460Status || 0x10 == hc32f460Status 
			 || 0x11 == hc32f460Status)
			return -1;
	
		memset(write_buf, 0, 128);
		printf("Package data...\n");
		printf("startAddress = %d\n", startAddress);
		write_len = Upgrade_Data_Code(CMD_ID_UpdateContinue, write_buf);
		printf("Package Done\nwrite_len = %d\n", write_len);
		
		rv = hc32f460_i2c_write(HC32F460_REG_FLASHADDR_0x03, write_buf, write_len);
		if(rv < 0)
		{
			printf("CMD_ID_UpdateContinue = failed!\n\n");
			return rv;
		}
		else
		{
			printf("CMD_ID_UpdateContinue = success!\n\n");
		}
		_sleep(20);
	}

	fclose(binaryFile);
	
	return 0;
}

int my_hc32f460_update(void)
{
	int rv;
	
	rv = hc32f460_update_start();
	if(rv < 0)
		return rv;
	_sleep(1800);
	
	rv = hc32f460_update_continue();
	if(rv < 0)
		return rv;

	rv = hc32f460_update_end();
	if(rv < 0)
		return rv;

	return 0;
}


int main(int argc, char *argv[])
{
    char IOInitOK=0;
    int i;
    int rv;

    IOInitOK = InitializeWinIo();
    if(IOInitOK)
    {
        printf("WinIo OK\n");
    }
    else
    {
        printf("Error during initialization of WinIo\n");
        goto IOError;
    }

    /* Init lpc */
    comm_init_lpc();

	read_hc32f460_version();
	read_hc32f460_status();
	printf("hc32f460Status = %02x\n\n", hc32f460Status);

	rv = get_firmware_size(argv[1]);
	if(rv < 0)
	{
		printf("get firmware size failed!\n");
		goto end;
	}

	rv = my_hc32f460_update();
	if(rv < 0)
	{
		printf("update hc32f460 firmware failed!\n");
		goto end;
	}
		
	_sleep(9000);
	read_hc32f460_version();
	read_hc32f460_status();
	printf("hc32f460Status = %02x\n\n", hc32f460Status);

    goto end;

IOError:
    ShutdownWinIo();
    return 1;
    
end:
ShutdownWinIo();
    return 0;
}
