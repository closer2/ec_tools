#define TOOLS_VER   "V0.2"
#define Vendor      "BITLAND"

//******************************************************************************
// WinFlash EC tool Version : 0.1
// 1. First Release
//    a. Update IT8987 eFlash (128K)
//    b. Update SPI flash (64K, 128K)
//******************************************************************************




/* Copyright (C)Copyright 2020 Bitland Telecom. All rights reserved.

   Author: MorgenZhu,ZhouHao
   
   Description: These functions of this file are reference only in the Windows!
   It can read/write chromium-EC RAM by 
   ----PM-port(62/66)
   ----KBC-port(60/64)
   ----EC-port(2E/2F or 4E/4F)
   ----Decicated I/O Port(301/302/303)
   
   
	Using VS2012 X86 cmd tool to compilation
	For windows-32/64bit
*/



//=============================Include file ====================================
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

#include "ec_commands.h"

using namespace std;
#include "winio.h"
//#pragma comment(lib,"WinIo.lib")       // For 32bit
#pragma comment(lib,"WinIox64.lib")      // For 64bit
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
//==============================================================================

//=================The hardware port to read/write function=====================
#define READ_PORT(port,data2)  GetPortVal(port, &data2, 1)
#define WRITE_PORT(port,data2) SetPortVal(port, data2, 1)

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
//==============================================================================



//======================== PM-1 channel for ACPI================================
WORD PM_STATUS_PORT66          =0x66;
WORD PM_CMD_PORT66             =0x66;
WORD PM_DATA_PORT62            =0x62;
#define PM_OBF                  0x01
#define PM_IBF                  0x02
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
void Send_cmd_by_PM(uint8_t Cmd)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_CMD_PORT66, Cmd);
    Wait_PM_IBE();
}

//------------send data by EC PM channel-----------------------/
void Send_data_by_PM(uint8_t Data)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_DATA_PORT62, Data);
    Wait_PM_IBE();
}

//-------------read data from EC PM channel--------------------/
uint8_t Read_data_from_PM(void)
{
    DWORD data;
    Wait_PM_OBF();
    READ_PORT(PM_DATA_PORT62, data);
    return(data);
}
//--------------write EC RAM-----------------------------------/
void EC_WriteByte_PM(uint8_t index, uint8_t data)
{
    Send_cmd_by_PM(0x81);
    Send_data_by_PM(index);
    Send_data_by_PM(data);
}
//--------------read EC RAM------------------------------------/
uint8_t EC_ReadByte_PM(uint8_t index)
{
    uint8_t data;
    Send_cmd_by_PM(0x80);
    Send_data_by_PM(index);
    data = Read_data_from_PM();
    return data;
}
//==============================================================================


//======================Console control interface===============================
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

void SetTextColor(uint8_t TextColor, uint8_t BackColor)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, (TextColor|(BackColor<<4)));
}

void SetPosition_X_Y(uint8_t PositionX, uint8_t PositionY)
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

#if 1
//========================= Chrome EC interface ================================
static int ec_readmem_lpc(int offset, int bytes, void *dest)
{
	int i = offset;
	char *s = (char *)dest;
	int cnt = 0;

	if (offset >= EC_MEMMAP_SIZE - bytes)
		return -1;

	if (bytes) {				/* fixed length */
		for (; cnt < bytes; i++, s++, cnt++)
			*s = inb(EC_LPC_ADDR_MEMMAP + i);
	} else {				/* string */
		for (; i < EC_MEMMAP_SIZE; i++, s++) {
			*s = inb(EC_LPC_ADDR_MEMMAP + i);
			cnt++;
			if (!*s)
				break;
		}
	}

	return cnt;
}


#define INITIAL_UDELAY 5     /* 5 us */
#define MAXIMUM_UDELAY 10000 /* 10 ms */
/* Don't use a macro where an inline will do... */
static inline int MIN(int a, int b) { return a < b ? a : b; }
static inline int MAX(int a, int b) { return a > b ? a : b; }

/* ec_command return value for non-success result from EC */
#define EECRESULT 1000

#define ec_command  ec_command_lpc_3

int ec_max_outsize, ec_max_insize;
void *ec_outbuf;
void *ec_inbuf;


/*
 * Wait for the EC to be unbusy.  Returns 0 if unbusy, non-zero if
 * timeout.
 */
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

static uint8_t read_mapped_mem8(uint8_t offset)
{
	int ret;
	uint8_t val;

	ret = ec_readmem_lpc(offset, sizeof(val), &val);
	if (ret <= 0) {
		fprintf(stderr, "failure in %s(): %d\n", __func__, ret);
		exit(1);
	}
	return val;
}

static uint16_t read_mapped_mem16(uint8_t offset)
{
	int ret;
	uint16_t val;

	ret = ec_readmem_lpc(offset, sizeof(val), &val);
	if (ret <= 0) {
		fprintf(stderr, "failure in %s(): %d\n", __func__, ret);
		exit(1);
	}
	return val;
}

static uint32_t read_mapped_mem32(uint8_t offset)
{
	int ret;
	uint32_t val;

	ret = ec_readmem_lpc(offset, sizeof(val), &val);
	if (ret <= 0) {
		fprintf(stderr, "failure in %s(): %d\n", __func__, ret);
		exit(1);
	}
	return val;
}

static int read_mapped_string(uint8_t offset, char *buffer, int max_size)
{
	int ret;

	ret = ec_readmem_lpc(offset, max_size, buffer);
	if (ret <= 0) {
		fprintf(stderr, "failure in %s(): %d\n", __func__, ret);
		exit(1);
	}
	return ret;
}

/**
 * Get the versions of the command supported by the EC.
 *
 * @param cmd		Command
 * @param pmask		Destination for version mask; will be set to 0 on
 *			error.
 * @return 0 if success, <0 if error
 */
int ec_get_cmd_versions(int cmd, uint32_t *pmask)
{
	struct ec_params_get_cmd_versions_v1 pver_v1;
	struct ec_params_get_cmd_versions pver;
	struct ec_response_get_cmd_versions rver;
	int rv;

	*pmask = 0;

	pver_v1.cmd = cmd;
	rv = ec_command(EC_CMD_GET_CMD_VERSIONS, 1, &pver_v1, sizeof(pver_v1),
			&rver, sizeof(rver));

	if (rv < 0) {
		pver.cmd = cmd;
		rv = ec_command(EC_CMD_GET_CMD_VERSIONS, 0, &pver, sizeof(pver),
				&rver, sizeof(rver));
	}

	if (rv < 0)
		return rv;

	*pmask = rver.version_mask;
	return 0;
}

/**
 * Return non-zero if the EC supports the command and version
 *
 * @param cmd		Command to check
 * @param ver		Version to check
 * @return non-zero if command version supported; 0 if not.
 */
int ec_cmd_version_supported(int cmd, int ver)
{
	uint32_t mask = 0;

	if (ec_get_cmd_versions(cmd, &mask))
		return 0;

	return (mask & EC_VER_MASK(ver)) ? 1 : 0;
}

int write_file(const char *filename, const uint8_t *buf, int size)
{
	FILE *f;
	int i;

	/* Write to file */
	f = fopen(filename, "wb");
	if (!f) {
		printf("Error opening output file %s", filename);
		return -1;
	}
	i = fwrite(buf, 1, size, f);
	fclose(f);
	if (i != size) {
        printf("Error writing to file %s", filename);
		return -1;
	}

	return 0;
}

char *read_file(const char *filename, int *size)
{
	FILE *f = fopen(filename, "rb");
	char *buf;
	int i;

	if (!f) {
        printf("Error opening output file %s", filename);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	rewind(f);
	if ((*size > 0x100000) || (*size < 0)) {
		if (*size < 0)
			perror("ftell failed");
		else
			fprintf(stderr, "File seems unreasonably large\n");
		fclose(f);
		return NULL;
	}

	buf = (char *)malloc(*size);
	if (!buf) {
		fprintf(stderr, "Unable to allocate buffer.\n");
		fclose(f);
		return NULL;
	}

	printf("Reading %d bytes from %s...\n", *size, filename);
	i = fread(buf, 1, *size, f);
	fclose(f);
	if (i != *size) {
		perror("Error reading file");
		free(buf);
		return NULL;
	}

	return buf;
}


//========================= Chrome EC interface end ============================
#endif

#if 1
//========================= Chrome EC command start ============================

/* Note: depends on enum ec_led_colors */
static const char * const led_color_names[] = {
	"red", "green", "blue", "yellow", "white", "amber"};
BUILD_ASSERT(ARRAY_SIZE(led_color_names) == EC_LED_COLOR_COUNT);

/* Note: depends on enum ec_led_id */
static const char * const led_names[] = {
	"battery", "power", "adapter", "left", "right", "recovery_hwreinit",
	"sysrq debug" };
BUILD_ASSERT(ARRAY_SIZE(led_names) == EC_LED_ID_COUNT);

int void_function(int argc, char *argv[])
{
    printf("Hello, this is bitland chrome ec tool, who are you?\n");
    return 0;
}


/* Note: depends on enum ec_image */
static const char * const image_names[] = {"unknown", "RO", "RW"};
#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]))


/******************************************************/
void sec_to_date(uint32_t sec, char *date)
{
	struct tm *time;
	time_t secs;
	secs = sec;
	time = localtime(&secs);

	strcpy(date, asctime(time));
	
	// delete '\n'
	date[strlen(date) - 1] = '\0';
}

void analysis_data(char *date)
{
	int i, j = 0;
	char str[32];

	strcpy(str, date);

	//year
	for(i = 20; i < 24; i++)
	{
		date[j++] = str[i];
	}
	date[j++] = '/';

	//month
	if(!strncmp(str+4, "Jan", 3))
	{
		date[j++] = '0';
		date[j++] = '1';		
	}
	else if(!strncmp(str+4, "Feb", 3))
	{
		date[j++] = '0';
		date[j++] = '2';
	}
	else if(!strncmp(str+4, "Mar", 3))
	{
		date[j++] = '0';
		date[j++] = '3';
	}
	else if(!strncmp(str+4, "Apr", 3))
	{
		date[j++] = '0';
		date[j++] = '4';
	}
	else if(!strncmp(str+4, "May", 3))
	{
		date[j++] = '0';
		date[j++] = '5';
	}
	else if(!strncmp(str+4, "Jun", 3))
	{
		date[j++] = '0';
		date[j++] = '6';
	}
	else if(!strncmp(str+4, "Jul", 3))
	{
		date[j++] = '0';
		date[j++] = '7';
	}
	else if(!strncmp(str+4, "Aug", 3))
	{
		date[j++] = '0';
		date[j++] = '8';
	}
	else if(!strncmp(str+4, "Sep", 3))
	{
		date[j++] = '0';
		date[j++] = '9';
	}
	else if(!strncmp(str+4, "Oct", 3))
	{
		date[j++] = '1';
		date[j++] = '0';
	}
	else if(!strncmp(str+4, "Nov", 3))
	{
		date[j++] = '1';
		date[j++] = '1';
	}
	else if(!strncmp(str+4, "Dec", 3))
	{
		date[j++] = '1';
		date[j++] = '2';
	}
	date[j++] = '/';

	//data
	for(i = 8; i < 10; i++)
	{
		if(str[i] == ' ')
		{
			date[j++] = '0';
			continue;
		}
		date[j++] = str[i];
	}
	date[j++] = ' ';

	for(i = 11; i < 19; i++)
	{
		date[j++] = str[i];
	}
	date[j] = '\0';
}


/******************************************************/


FILE *SDCfgFile = NULL;
FILE *WUCfgFile = NULL;
FILE *SDTxtFile = NULL;
FILE *WUTxtFile = NULL;
FILE *txtFile   = NULL;
FILE *binaryFile =NULL;



typedef struct Log_Info{
	uint32_t log_id;
	char log[128];
}log_info_struct;
	
log_info_struct shutdown_case_list[]=
{
	{0x01,	"S0 to S4/S5 normal shutdown"},
	{0x02,	"S3 to S4/S5 fail off"},
	{0x03,	"SLP_S3 low trig S0 to S3"},
	{0x04,	"SLP_S4 low trig S0 to S4"},
	{0x05,	"SLP_S5 low trig S0 to S5(SLP_S4/S5 use 0x04)"},
	{0x06,	"Power button 4s seconds overwrite"},
	{0x07,	"Power button 10s seconds overwrite"},
	{0x08,	"HWPG fail in S0"},
	{0x09,	"Power on fail NO.1(SX to S0)  Power on WDT"},
	{0x0A,	"Power on fail NO.1(SX to S0)  Power on WDT by HWPG"},
	{0x0B,	"Power on fail NO.2(SX to S0)  SLP_S3(SUSB) fail"},
	{0x0C,	"Power on fail NO.3(SX to S0)  SLP_S4(SUSC) fail"},
	{0x0D,	"Power on fail NO.4(SX to S0)  SLP_S5 fail"},
	{0x0E,	"Power on fail NO.5(SX to S0)  HWPG fail"},
	{0x0F,	"Power on fail NO.6(SX to S0)  RSMRST fail"},
	{0x10,	"Power on fail NO.6(SX to S0)  PLTRST fail"},

	{0x20,	"PMIC reset by voltage regulator fault(PMIC reset IRQ)"},
	{0x21,	"PMIC reset by power button counter (PMIC reset IRQ)"},
	{0x2E,	"Caterr  low trig Shutdown"},
	
	{0x30,	"CPU too hot(PECI)"},
	{0x31,	"CPU too hot(Thermistor: CPU)"},
	{0x32,	"VGA too hot"},
	{0x33,	"SYS too hot(Thermal sensor:Local)"},
	{0x34,	"PCH too hot(Thermal sensor:Remote)"},
	{0x35,	"DDR too hot(Thermistor: DDR)"},
	{0x36,	"DCJ too hot(Thermistor: DC Jack)"},
	{0x37,	"Ambient too hot(Thermistor: Ambient)"},
	{0x38,	"SSD too hot(Thermistor:SSD)"},
	
	{0xD0,	"BIOS/OS WDT"},
	{0xD1,	"BIOS bootbloack fail"},
	{0xD2,	"BIOS Memory_initila Fail"},
	{0xD3,	"BIOS main block fail"},
	{0xD4,	"BIOS Crisis Fail"},
	{0xD7,	"Flash BIOS Start BIOS"},
	{0xD8,	"Flash BIOS End BIOS"},
	
	{0xEB,	"EC RAM initial fail"},
	{0xEC,	"EC code reset"},
	{0xFC,	"EC VSTBY or WRST reset"},
	{0xFD,	"EC external WDT"},
	{0xFE,	"EC internal WDT"},
};

log_info_struct wakeup_case_list[]=
{
	{0x01,	"PWRSW wakeup from S4/S5(interrupt under AC mode)"},
	{0x02,	"PWRSW wakeup from S4/S5(polling under DC mode)"},
	{0x03,	"PWRSW wakeup from S3"},
	{0x04,	"SLP_S3 high trig S3 to S0"},
	{0x05,	"SLP_S4 high trig S4 to S0"},
	{0x06,	"SLP_S5 high trig S5 to S0 (SLP_S4/S5 use 0x04)"},
	
	{0xD1,	"Wakeup from BIOS notify EC after BIOS update"},
	{0xD2,	"BIOS autowake from S3"},
	{0xD3,	"BIOS autowake from S4"},
	{0xD4,	"BIOS autowake from S5(SLP_S4/S5 use SLP_S4)"},

	{0xFC,	"EC auto power on"},
	{0xFD,	"EC mirror code auto on"},
	{0xFE,	"EC internal watchdog reset cause wakeup"},
};
	

/*****************************************/


typedef struct Shutdown_Wakeup_Case{
	uint32_t log_id;
	char log[128];
	struct Shutdown_Wakeup_Case *next;
}Node;

static Node *shutdown_case_head = NULL;
static Node *shutdown_case_tail= NULL;
static Node *wakeup_case_head = NULL;
static Node *wakeup_case_tail= NULL;

void add_data_to_linklist(Node **head, Node **tail, uint32_t log_id, char *log)
{
	Node *new_node = NULL;
	new_node = (Node *)malloc(sizeof(Node));
	new_node->log_id = log_id;
	strcpy(new_node->log, log);
	new_node->next = NULL;
	if(*head == NULL || *tail == NULL)
	{
		*head = new_node;
		*tail = new_node;
	}
	else
	{
		(*tail)->next = new_node;
		*tail = new_node;
	}
}

void printlist(Node *ptr)
{
	while(ptr)
	{
		printf(" 0x%02x\t", ptr->log_id);
		printf("%s\n", ptr->log);
		ptr = ptr->next;
	}
}

void free_linklist(Node *head)
{
	Node *ptr, *ptr2;
	ptr = head;
	while(ptr)
	{
		ptr2 = ptr;
		ptr = ptr->next;
		free(ptr2);	
	}
}

int analysis_log_info(Node *head, uint32_t log_id, FILE *txtFile)
{
	Node *ptr;
	ptr = head;
	while(ptr)
	{
		if(ptr->log_id == log_id)
		{
			fprintf(txtFile, "%s\n", ptr->log);
			return 0;
		}
		ptr = ptr->next;
	}
	return -1;
}

void creat_shutdown_case_cfgFile(void)
{
    unsigned int i;
	int log_num;
    
    SDCfgFile = fopen("ShutdownCase.cfg","w");
    if(SDCfgFile == NULL)
    {
        printf("Creat default ShutdownCase.cfg file Fail\n\n");
        return;
    }
    
    fprintf(SDCfgFile, "[ShutdownCase Version=%s]\n", TOOLS_VER);
    fprintf(SDCfgFile, "#************************************************************\n");
    fprintf(SDCfgFile, "# This shutdown case config file\n");
    fprintf(SDCfgFile, "# Shutdown case start with $# \n");
    fprintf(SDCfgFile, "# You can add id and Shutdown case according to the rules\n");
    fprintf(SDCfgFile, "# Author : \n");
    fprintf(SDCfgFile, "#************************************************************\n\n\n");

	log_num = sizeof(shutdown_case_list)/sizeof(log_info_struct);
	for (i = 0; i < log_num; ++i)
	{
		fprintf(SDCfgFile, "$#");
        fprintf(SDCfgFile, "0x%02x", shutdown_case_list[i].log_id);
        fprintf(SDCfgFile, "\t#");
        fprintf(SDCfgFile, "%s#\n", shutdown_case_list[i].log);
	}

	fprintf(SDCfgFile, "\n");
    fclose(SDCfgFile);
    printf("Creat default ShutdownCase.cfg file OK\n\n");
}

int read_shutdown_case_into_linklist(void)
{
    char StrLine[1024];
    char *pStrLine;
    log_info_struct *log_date;
    char *str;
    int i;
	char log_id_buf[64];

	log_date = (log_info_struct *)malloc(sizeof(log_info_struct));
    
    if((SDCfgFile = fopen("ShutdownCase.cfg","r")) == NULL)
    {
        printf("ShutdownCase.cfg not exist\n\n");
        
        // Craet eFlashDebug config file
        creat_shutdown_case_cfgFile();
        return -1;
    }


    while (!feof(SDCfgFile))
    {   
        // Read one line data
        fgets(StrLine, 1024, SDCfgFile);
        pStrLine = StrLine;
		
        if(('$'==StrLine[0]) && (('#'==StrLine[1])))
        {
            pStrLine = pStrLine + 2;
            i = 0;
            while(('#' != (*pStrLine)))
            {
               log_id_buf[i] = *pStrLine;
               i++;
               pStrLine++;
            }
			log_date->log_id = (uint32_t)strtoul(log_id_buf, &str, 16);
			
            while(('#' != (*pStrLine++)));
            i = 0;
            while(('#' != (*pStrLine)))
            {
               log_date->log[i] = *pStrLine;
               i++;
               pStrLine++;
            }
            log_date->log[i] = '\0';

			//add log info into linklist
			add_data_to_linklist(&shutdown_case_head, &shutdown_case_tail, 
							log_date->log_id, log_date->log);
        }
    }
	return 0;
}


void creat_wakeup_case_cfgFile(void)
{
    unsigned int i;
	int log_num;
    
    WUCfgFile = fopen("WakeupCase.cfg","w");
    if(WUCfgFile == NULL)
    {
        printf("Creat default WakeupCase.cfg file Fail\n\n");
        return;
    }
    
    fprintf(WUCfgFile, "[WakeupCase Version=%s]\n", TOOLS_VER);
    fprintf(WUCfgFile, "#************************************************************\n");
    fprintf(WUCfgFile, "# This wakeup case config file\n");
    fprintf(WUCfgFile, "# Wakeup case start with $# \n");
    fprintf(WUCfgFile, "# You can add id and wakeup case according to the rules\n");
    fprintf(WUCfgFile, "# Author : \n");
    fprintf(WUCfgFile, "#************************************************************\n\n\n");

	log_num = sizeof(wakeup_case_list)/sizeof(log_info_struct);
	for (i = 0; i < log_num; ++i)
	{
		fprintf(WUCfgFile, "$#");
        fprintf(WUCfgFile, "0x%02x", wakeup_case_list[i].log_id);
        fprintf(WUCfgFile, "\t#");
        fprintf(WUCfgFile, "%s#\n", wakeup_case_list[i].log);
	}

	fprintf(WUCfgFile, "\n");
    fclose(WUCfgFile);
    printf("Creat default WakeupCase.cfg file OK\n\n");
}

int read_wakeup_case_into_linklist(void)
{
    char StrLine[1024];
	char *pStrLine;
    log_info_struct *log_date;
    char *str;
    int i;
	char log_id_buf[64];

	log_date = (log_info_struct *)malloc(sizeof(log_info_struct));
    
    if((WUCfgFile = fopen("WakeupCase.cfg","r")) == NULL)
    {
        printf("WakeupCase.cfg not exist\n\n");
        
        // Craet eFlashDebug config file
        creat_wakeup_case_cfgFile();
        return -1;
    }

    while (!feof(WUCfgFile))
    {   
        // Read one line data
        fgets(StrLine, 1024, WUCfgFile);
        
        pStrLine = StrLine;
        if(('$'==StrLine[0]) && (('#'==StrLine[1])))
        {
            pStrLine = pStrLine + 2;
			i = 0;
            while(('#' != (*pStrLine)))
            {
               log_id_buf[i] = *pStrLine;
               i++;
               pStrLine++;
            }
			log_date->log_id = (uint32_t)strtoul(log_id_buf, &str, 16);
            
            while(('#' != (*pStrLine++)));
            i = 0;
            while(('#' != (*pStrLine)))
            {
               log_date->log[i] = *pStrLine;
               i++;
               pStrLine++;
            }
            log_date->log[i] = '\0';

			//add log info into linklist
			add_data_to_linklist(&wakeup_case_head, &wakeup_case_tail, 
							log_date->log_id, log_date->log);
        }
    }
	return 0;
}


/******************************************************/


typedef struct Log_Node{
	uint32_t sec;
	uint32_t log_id;
	uint32_t flag;
	struct Log_Node *prew;
	struct Log_Node *next;
}LogNode;

static LogNode *raw_log_head = NULL;
static LogNode *raw_log_tail = NULL;

static LogNode *seq_log_head = NULL;
static LogNode *seq_log_tail = NULL;

void add_data_from_head(LogNode **head, LogNode **tail, uint32_t sec, uint32_t log_id)
{
	LogNode *new_node = NULL;
	new_node = (LogNode *)malloc(sizeof(LogNode));
	new_node->sec = sec;
	new_node->log_id = log_id;
	new_node->prew = NULL;
	new_node->next = NULL;
	if(*head == NULL || *tail == NULL)
	{
		*head = new_node;
		*tail = new_node;
	}
	else
	{
		(*head)->prew = new_node;
		new_node->next = *head;
		*head = new_node;
	}
}

void add_data_from_tail(LogNode **head, LogNode **tail, uint32_t sec, uint32_t log_id, uint32_t flag)
{
	LogNode *new_node = NULL;
	new_node = (LogNode *)malloc(sizeof(LogNode));
	new_node->sec = sec;
	new_node->log_id = log_id;
	new_node->flag = flag;
	new_node->prew = NULL;
	new_node->next = NULL;
	if(*head == NULL || *tail == NULL)
	{
		*head = new_node;
		*tail = new_node;
	}
	else
	{
		(*tail)->next = new_node;
		new_node->prew = *tail;
		*tail = new_node;
	}
}

void insert_data_to_seq_linklist(LogNode *ptr, uint32_t sec, uint32_t log_id, uint32_t flag)
{
	LogNode *new_node = NULL;
	new_node = (LogNode *)malloc(sizeof(LogNode));
	new_node->sec = sec;
	new_node->log_id = log_id;
	new_node->flag = flag;
	new_node->prew = NULL;
	new_node->next = NULL;
	if(ptr->prew == NULL)
	{
		new_node->next = seq_log_head;
		seq_log_head->prew = new_node;
		seq_log_head = new_node;
	}
	else
	{
		new_node->next = ptr;
		new_node->prew = ptr->prew;
		ptr->prew->next = new_node;
		ptr->prew = new_node;
	}
}


void my_printlist(LogNode *ptr)
{
	while(ptr)
	{
		printf(" sec = %x\t", ptr->sec);
		printf("log_id = %x\n", ptr->log_id);
		ptr = ptr->next;
	}
	printf("******************\n\n");
}

void sort_raw_log_linklist(LogNode *raw)
{
	LogNode *ptr;
	while(raw)
	{
		ptr = seq_log_head;
		if(seq_log_head == NULL)
		{
			add_data_from_tail(&seq_log_head, &seq_log_tail, raw->sec, raw->log_id, raw->flag);
		}
		
		while(ptr)
		{
			if(raw->sec < ptr->sec)
			{
				//printf("insert---\nraw->sec = %x\traw->log_id = %x\n",  raw->sec, raw->log_id);
				insert_data_to_seq_linklist(ptr, raw->sec, raw->log_id, raw->flag);
				break;
			}
			else if(ptr->next == NULL)
			{
				//printf("add---\nraw->sec = %x\traw->log_id = %x\n",  raw->sec, raw->log_id);
				add_data_from_tail(&seq_log_head, &seq_log_tail, raw->sec, raw->log_id, raw->flag);
				break;
			}
			else
			{
				ptr = ptr->next;
			}
		}
		raw = raw->next;
	}
}

void analysis_seq_linklist(LogNode *ptr)
{
	char date[32];
	while(ptr)
	{
		//analysis data
		sec_to_date(ptr->sec, date);
		analysis_data(date);
		fprintf(txtFile, "[%s] : ID=[%02X], ", date, ptr->log_id);

		//analysis log
		if(1 == ptr->flag)
		{
			if( analysis_log_info(shutdown_case_head, ptr->log_id, txtFile) == 0)
			{}
			else
			{
				fprintf(txtFile, "unkonw log id\n");
			}
		}

		if(0 == ptr->flag)
		{
			if( analysis_log_info(wakeup_case_head, ptr->log_id, txtFile) == 0)
			{}
			else
			{
				fprintf(txtFile, "unkonw log id\n");
			}
		}

		//next
		ptr = ptr->next;
	}
}


/*****************************************/

int cmd_version(int argc, char *argv[])
{
	struct ec_response_get_version r;
	char build_string[256];
	int rv;

	rv = ec_command(EC_CMD_GET_VERSION, 0, NULL, 0, &r, sizeof(r));
	if (rv < 0)
    {
		fprintf(stderr, "ERROR: EC_CMD_GET_VERSION failed: %d\n", rv);
		goto exit;
	}
	rv = ec_command(EC_CMD_GET_BUILD_INFO, 0,
			NULL, 0, &build_string, EC_PROTO2_MAX_PARAM_SIZE);
	if (rv < 0)
    {
		fprintf(stderr, "ERROR: EC_CMD_GET_BUILD_INFO failed: %d\n", rv);
		goto exit;
	}
	rv = 0;

	/* Ensure versions are null-terminated before we print them */
	r.version_string_ro[31] = '\0';
	r.version_string_rw[31] = '\0';
	build_string[255] = '\0';

	/* Print versions */
	printf(" RO version              : %s\n", r.version_string_ro);
	printf(" RW version              : %s\n", r.version_string_rw);
	printf(" Firmware copy           : %s\n",
	       (r.current_image < ARRAY_SIZE(image_names) ?
		    image_names[r.current_image] : "?"));
	printf(" Build info              : %s\n\n", build_string);
exit:
	printf(" BITLAND EC Tool version : %s\n\n", TOOLS_VER);

	return rv;
}


int cmd_board_version(int argc, char *argv[])
{
	struct ec_response_board_version response;
	int rv=0;

	rv = ec_command_lpc_3(EC_CMD_GET_BOARD_VERSION, 0, NULL, 0, &response,
			sizeof(response));
	if (rv < 0)
		return rv;

	printf("Project version	 : %d\n\n", response.project_version);
	printf("Board version    : %d\n\n", response.board_version);

	return rv;
}


int cmd_adc_read(int argc, char *argv[])
{
	char *e;
	struct ec_params_adc_read p;
	struct ec_response_adc_read r;
	int rv;

	if (argc < 2)
    {
		fprintf(stderr, "Usage: %s <adc channel>\n", argv[0]);
		return -1;
	}

	p.adc_channel = (uint8_t)strtoull(argv[1], &e, 0);
	if (e && *e)
    {
		fprintf(stderr, "\"%s\": invalid channel!\n", argv[1]);
		return -1;
	}

	rv = ec_command(EC_CMD_ADC_READ, 0, &p, sizeof(p), &r, sizeof(r));
	if (rv > 0)
    {
		printf("ADC channel-%s  : %d\n\n", argv[1], r.adc_value);
		return 0;
	}
	return rv;
}

int cmd_apreset(int argc, char *argv[])
{
	return ec_command(EC_CMD_AP_RESET, 0, NULL, 0, NULL, 0);
}

static int get_num_fans(void)
{
	int idx, rv;
	struct ec_response_get_features r;

	/*
	 * iff the EC supports the GET_FEATURES,
	 * check whether it has fan support enabled.
	 */
	rv = ec_command(EC_CMD_GET_FEATURES, 0, NULL, 0, &r, sizeof(r));
	if (rv >= 0 && !(r.flags[0] & BIT(EC_FEATURE_PWM_FAN)))
		return 0;

	for (idx = 0; idx < EC_FAN_SPEED_ENTRIES; idx++) {
		rv = read_mapped_mem16(EC_MEMMAP_FAN_RPM + 2 * idx);
		if (rv == EC_FAN_SPEED_NOT_PRESENT)
			break;
	}

	return idx;
}

int cmd_thermal_auto_fan_ctrl(int argc, char *argv[])
{
	int rv, num_fans;
	struct ec_params_auto_fan_ctrl_v1 p_v1;
	char *e;
	int cmdver = 1;

	if (!ec_cmd_version_supported(EC_CMD_THERMAL_AUTO_FAN_CTRL, cmdver)
	    || (argc == 1)) {
		/* If no argument is provided then enable auto fan ctrl */
		/* for all fans by using version 0 of the host command */

		rv = ec_command(EC_CMD_THERMAL_AUTO_FAN_CTRL, 0,
				NULL, 0, NULL, 0);
		if (rv < 0)
			return rv;

		printf("Automatic fan control is now on for all fans.\n");
		return 0;
	}

	if (argc > 2 || !strcmp(argv[1], "help")) {
		printf("Usage: %s [idx]\n", argv[0]);
		return -1;
	}

	num_fans = get_num_fans();
	p_v1.fan_idx = strtol(argv[1], &e, 0);
	if ((e && *e) || (p_v1.fan_idx >= num_fans)) {
		fprintf(stderr, "Bad fan index.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_THERMAL_AUTO_FAN_CTRL, cmdver,
			&p_v1, sizeof(p_v1), NULL, 0);
	if (rv < 0)
		return rv;

	printf("Automatic fan control is now on for fan %d\n", p_v1.fan_idx);

	return 0;
}

static int print_fan(int idx)
{
	int rv = read_mapped_mem16(EC_MEMMAP_FAN_RPM + 2 * idx);

	switch (rv) {
	case EC_FAN_SPEED_NOT_PRESENT:
		return -1;
	case EC_FAN_SPEED_STALLED:
		printf("Fan %d stalled!\n", idx);
		break;
	default:
		printf("Fan %d RPM: %d\n", idx, rv);
		break;
	}

	return 0;
}

int cmd_pwm_get_num_fans(int argc, char *argv[])
{
	int num_fans;

	num_fans = get_num_fans();

	printf("Number of fans = %d\n", num_fans);

	return 0;
}

int cmd_pwm_get_fan_rpm(int argc, char *argv[])
{
	int i, num_fans;

	num_fans = get_num_fans();
	if (argc < 2 || !strcmp(argv[1], "all")) {
		/* Print all the fan speeds */
		for (i = 0; i < num_fans; i++)
			print_fan(i);
	} else {
		char *e;
		int idx;

		idx = strtol(argv[1], &e, 0);
		if ((e && *e) || idx < 0 || idx >= num_fans) {
			fprintf(stderr, "Bad index.\n");
			return -1;
		}

		print_fan(idx);
	}

	return 0;
}

int cmd_pwm_set_fan_rpm(int argc, char *argv[])
{
	struct ec_params_pwm_set_fan_target_rpm_v1 p_v1;
	char *e;
	int rv, num_fans;
	int cmdver = 1;

	if (!ec_cmd_version_supported(EC_CMD_PWM_SET_FAN_TARGET_RPM, cmdver)) {
		struct ec_params_pwm_set_fan_target_rpm_v0 p_v0;

		/* Fall back to command version 0 command */
		cmdver = 0;

		if (argc != 2) {
			fprintf(stderr,
				"Usage: %s <targetrpm>\n", argv[0]);
			return -1;
		}
		p_v0.rpm = strtol(argv[1], &e, 0);
		if (e && *e) {
			fprintf(stderr, "Bad RPM.\n");
			return -1;
		}

		rv = ec_command(EC_CMD_PWM_SET_FAN_TARGET_RPM, cmdver,
				&p_v0, sizeof(p_v0), NULL, 0);
		if (rv < 0)
			return rv;

		printf("Fan target RPM set for all fans.\n");
		return 0;
	}

	if (argc > 3 || (argc == 2 && !strcmp(argv[1], "help")) || argc == 1) {
		printf("Usage: %s [idx] <targetrpm>\n", argv[0]);
		printf("'%s 0 3000' - Set fan 0 RPM to 3000\n", argv[0]);
		printf("'%s 3000' - Set all fans RPM to 3000\n", argv[0]);
		return -1;
	}

	num_fans = get_num_fans();
	p_v1.rpm = strtol(argv[argc - 1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad RPM.\n");
		return -1;
	}

	if (argc == 2) {
		/* Reuse version 0 command if we're setting targetrpm
		 * for all fans */
		struct ec_params_pwm_set_fan_target_rpm_v0 p_v0;

		cmdver = 0;
		p_v0.rpm = p_v1.rpm;

		rv = ec_command(EC_CMD_PWM_SET_FAN_TARGET_RPM, cmdver,
				&p_v0, sizeof(p_v0), NULL, 0);
		if (rv < 0)
			return rv;

		printf("Fan target RPM set for all fans.\n");
	} else {
		p_v1.fan_idx = strtol(argv[1], &e, 0);
		if ((e && *e) || (p_v1.fan_idx >= num_fans)) {
			fprintf(stderr, "Bad fan index.\n");
			return -1;
		}

		rv = ec_command(EC_CMD_PWM_SET_FAN_TARGET_RPM, cmdver,
				&p_v1, sizeof(p_v1), NULL, 0);
		if (rv < 0)
			return rv;

		printf("Fan %d target RPM set.\n", p_v1.fan_idx);
	}

	return 0;
}

int cmd_fanduty(int argc, char *argv[])
{
	struct ec_params_pwm_set_fan_duty_v1 p_v1;
	char *e;
	int rv, num_fans;
	int cmdver = 1;

	if (!ec_cmd_version_supported(EC_CMD_PWM_SET_FAN_DUTY, cmdver)) {
		struct ec_params_pwm_set_fan_duty_v0 p_v0;

		if (argc != 2) {
			fprintf(stderr,
				"Usage: %s <percent>\n", argv[0]);
			return -1;
		}
		p_v0.percent = strtol(argv[1], &e, 0);
		if (e && *e) {
			fprintf(stderr, "Bad percent arg.\n");
			return -1;
		}

		rv = ec_command(EC_CMD_PWM_SET_FAN_DUTY, 0,
				&p_v0, sizeof(p_v0), NULL, 0);
		if (rv < 0)
			return rv;

		printf("Fan duty cycle set.\n");
		return 0;
	}

	if (argc > 3 || (argc == 2 && !strcmp(argv[1], "help")) || argc == 1) {
		printf("Usage: %s [idx] <percent>\n", argv[0]);
		printf("'%s 0 50' - Set fan 0 duty cycle to 50 percent\n",
			argv[0]);
		printf("'%s 30' - Set all fans duty cycle to 30 percent\n",
			argv[0]);
		return -1;
	}

	num_fans = get_num_fans();
	p_v1.percent = strtol(argv[argc - 1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad percent arg.\n");
		return -1;
	}

	if (argc == 2) {
		/* Reuse version 0 command if we're setting duty cycle
		 * for all fans */
		struct ec_params_pwm_set_fan_duty_v0 p_v0;

		cmdver = 0;
		p_v0.percent = p_v1.percent;

		rv = ec_command(EC_CMD_PWM_SET_FAN_DUTY, cmdver,
				&p_v0, sizeof(p_v0), NULL, 0);
		if (rv < 0)
			return rv;

		printf("Fan duty cycle set for all fans.\n");
	} else {
		p_v1.fan_idx = strtol(argv[1], &e, 0);
		if ((e && *e) || (p_v1.fan_idx >= num_fans)) {
			fprintf(stderr, "Bad fan index.\n");
			return -1;
		}

		rv = ec_command(EC_CMD_PWM_SET_FAN_DUTY, cmdver,
				&p_v1, sizeof(p_v1), NULL, 0);
		if (rv < 0)
			return rv;

		printf("Fan %d duty cycle set.\n", p_v1.fan_idx);
	}

	return 0;
}

int is_string_printable(const char *buf)
{
	while (*buf) {
		if (!isprint(*buf))
			return 0;
		buf++;
	}
	return 1;
}

int cmd_battery(int argc, char *argv[])
{
	#if 0
	char batt_text[EC_MEMMAP_TEXT_MAX];
	int rv, val;
	char *e;
	int index = 0;

    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [index]\n", argv[0]);
        return -1;
    }
    else if (argc == 2)
    {
		index = strtol(argv[1], &e, 0);
		if (e && *e)
        {
			fprintf(stderr, "Bad battery index.\n");
			return -1;
		}
	}

	val = read_mapped_mem8(EC_MEMMAP_BATTERY_VERSION);
	if (val < 1)
    {
		fprintf(stderr, "Battery version %d is not supported\n", val);
		return -1;
	}

	printf("\nBattery info:\n");

	rv = read_mapped_string(EC_MEMMAP_BATT_MFGR, batt_text, sizeof(batt_text));
	if (rv < 0 || !is_string_printable(batt_text))
		goto cmd_error;
	printf("  OEM name           : %s\n", batt_text);

	rv = read_mapped_string(EC_MEMMAP_BATT_MODEL, batt_text, sizeof(batt_text));
	if (rv < 0 || !is_string_printable(batt_text))
		goto cmd_error;
	printf("  Model number       : %s\n", batt_text);

	rv = read_mapped_string(EC_MEMMAP_BATT_TYPE, batt_text, sizeof(batt_text));
	if (rv < 0 || !is_string_printable(batt_text))
		goto cmd_error;
	printf("  Chemistry          : %s\n", batt_text);

	rv = read_mapped_string(EC_MEMMAP_BATT_SERIAL, batt_text, sizeof(batt_text));
	printf("  Serial number      : %s\n", batt_text);

	val = read_mapped_mem32(EC_MEMMAP_BATT_DCAP);
	if((val<0) || (val>65535))
		goto cmd_error;
	printf("  Design capacity    : %u mAh\n", val);

	val = read_mapped_mem32(EC_MEMMAP_BATT_LFCC);
	if((val<0) || (val>65535))
		goto cmd_error;
	printf("  Last full charge   : %u mAh\n", val);

	val = read_mapped_mem32(EC_MEMMAP_BATT_DVLT);
	if((val<0) || (val>65535))
		goto cmd_error;
	printf("  Design voltage     : %u mV\n", val);

	val = read_mapped_mem32(EC_MEMMAP_BATT_CCNT);
	if((val<0) || (val>65535))
		goto cmd_error;
	printf("  Cycle count        : %u\n", val);

	val = read_mapped_mem32(EC_MEMMAP_BATT_VOLT);
	if((val<0) || (val>65535))
		goto cmd_error;
	printf("  Present voltage    : %u mV\n", val);

	val = read_mapped_mem32(EC_MEMMAP_BATT_RATE);
	if((val<0) || (val>65535))
		goto cmd_error;
	printf("  Present current    : %u mA\n", val);

	val = read_mapped_mem32(EC_MEMMAP_BATT_CAP);
	if((val<0) || (val>65535))
		goto cmd_error;
	printf("  Remaining capacity : %u mAh\n", val);

	val = read_mapped_mem8(EC_MEMMAP_BATT_FLAG);
    printf("  Flags              : [0x%02x] ", val);

    printf("%s",((val & EC_BATT_FLAG_AC_PRESENT)?("AC_IN/"):("AC_OUT/")));
    printf("%s",((val & EC_BATT_FLAG_BATT_PRESENT)?("BAT_IN/"):("BAT_OUT/")));
    printf("%s",((val & EC_BATT_FLAG_DISCHARGING)?("DISCHARGING/"):("")));
    printf("%s",((val & EC_BATT_FLAG_CHARGING)?("CHARGING/"):("")));
    printf("%s",((val & EC_BATT_FLAG_LEVEL_CRITICAL)?("LEVEL_CRITICAL/"):("")));
	printf("\n\n");

	return 0;
cmd_error:
	fprintf(stderr, "Bad battery info value. Check protocol version.\n\n");
	return -1;
	#endif
	return -1;
}

int cmd_battery_cut_off(int argc, char *argv[])
{
	struct ec_params_battery_cutoff p;
	int cmd_version;
	int rv;

	memset(&p, 0, sizeof(p));
	if (ec_cmd_version_supported(EC_CMD_BATTERY_CUT_OFF, 1))
    {
		cmd_version = 1;
		if (argc > 1)
        {
			if (!strcmp(argv[1], "at-shutdown"))
            {
				p.flags = EC_BATTERY_CUTOFF_FLAG_AT_SHUTDOWN;
			}
            else
            {
				fprintf(stderr, "Bad parameter: %s\n", argv[1]);
				return -1;
			}
		}
	}
    else
    {
		/* Fall back to version 0 command */
		cmd_version = 0;
		if (argc > 1)
        {
			if (!strcmp(argv[1], "at-shutdown"))
            {
				fprintf(stderr, "Explicit 'at-shutdown' ");
				fprintf(stderr, "parameter not supported.\n");
			}
            else
            {
				fprintf(stderr, "Bad parameter: %s\n", argv[1]);
			}
			return -1;
		}
	}

	rv = ec_command(EC_CMD_BATTERY_CUT_OFF, cmd_version, &p, sizeof(p), NULL, 0);
	rv = (rv < 0 ? rv : 0);

	if (rv < 0)
    {
		fprintf(stderr, "Failed to cut off battery, rv=%d\n", rv);
		fprintf(stderr, "It is expected if the rv is -%d "
				"(EC_RES_INVALID_COMMAND) if the battery "
				"doesn't support cut-off function.\n",
				EC_RES_INVALID_COMMAND);
	}
    else
    {
		printf("\n");
		printf("SUCCESS. The battery has arranged a cut-off.\n");

		if (cmd_version == 1 &&
		    (p.flags & EC_BATTERY_CUTOFF_FLAG_AT_SHUTDOWN))
			printf("The battery will be cut off after shutdown.\n");
		else
			printf("The system should be shutdown immediately.\n");

		printf("\n\n");
	}
	return rv;
}

int cmd_switches(int argc, char *argv[])
{
	struct ec_switch_funtion p;
	int i, rv;
	char *e;

	if(argc != 3)
	{
		fprintf(stderr,
			"Usage: %s <type> <switch>\n", argv[0]);
	}

	if(!strcmp("powerled", argv[1]))
		p.type = 0x01;
	else if(!strcmp("wakeonlan", argv[1]))
		p.type = 0x02;
	else if(!strcmp("wakeonwlan", argv[1]))
		p.type = 0x03;
	else 
	{
		fprintf(stderr, "Bad type name: %s\n", argv[1]);
		fprintf(stderr, "Valid type names: powerled | "
						 "wakeonwlan | wakeonwlan\n");
		return -1;
	}
	
	if(!strcmp("on", argv[2]))
		p.switchi = 0x01;
	else if(!strcmp("off", argv[2]))
		p.switchi = 0x02;
	else
	{
		fprintf(stderr, "Bad switchi name: %s\n", argv[1]);
		fprintf(stderr, "Valid switchi names: on | off\n");
		return -1;
	}

	rv = ec_command(EC_CMD_SWITCH_FUNTION, 0, &p, sizeof(p), NULL, 0);
	if (rv < 0)
			return rv;

	return 0;
}

int cmd_chipinfo(int argc, char *argv[])
{
	struct ec_response_get_chip_info info;
	int rv;

	printf("\nChip info :\n");

	rv = ec_command(EC_CMD_GET_CHIP_INFO, 0, NULL, 0, &info, sizeof(info));
	if (rv < 0)
		return rv;
	printf("  vendor      : %s\n", info.vendor);
	printf("  name        : %s\n", info.name);
	printf("  revision    : %s\n\n", info.revision);

	return 0;
}

int cmd_cmdversions(int argc, char *argv[])
{
	struct ec_params_get_cmd_versions p;
	struct ec_response_get_cmd_versions r;
	char *e;
	int cmd;
	int rv;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <cmd>\n", argv[0]);
		return -1;
	}
	cmd = strtol(argv[1], &e, 0);
	if ((e && *e) || cmd < 0 || cmd > 0xffff) {
		fprintf(stderr, "Bad command number.\n");
		return -1;
	}

	p.cmd = cmd;
	rv = ec_command(EC_CMD_GET_CMD_VERSIONS, 0, &p, sizeof(p),
			&r, sizeof(r));

	if (rv < 0) {
		if (rv == -EECRESULT - EC_RES_INVALID_PARAM)
			printf("Command 0x%02x not supported by EC.\n", cmd);

		return rv;
	}

	printf("Command 0x%02x supports version mask 0x%08x\n",
	       cmd, r.version_mask);
	return 0;
}

int cmd_console(int argc, char *argv[])
{
	char *out = (char *)ec_inbuf;
	int rv;

	/* Snapshot the EC console */
	rv = ec_command(EC_CMD_CONSOLE_SNAPSHOT, 0, NULL, 0, NULL, 0);
	if (rv < 0)
		return rv;

	/* Loop and read from the snapshot until it's done */
	while (1) {
		rv = ec_command(EC_CMD_CONSOLE_READ, 0,
				NULL, 0, ec_inbuf, ec_max_insize);
		if (rv < 0)
			return rv;

		/* Empty response means done */
		if (!rv || !*out)
			break;

		/* Make sure output is null-terminated, then dump it */
		out[ec_max_insize - 1] = '\0';
		fputs(out, stdout);
	}
	printf("\n");
	return 0;
}

int cmd_host_event_get_raw(int argc, char *argv[])
{
	uint32_t events = read_mapped_mem32(EC_MEMMAP_HOST_EVENTS);

	if (events & EC_HOST_EVENT_MASK(EC_HOST_EVENT_INVALID)) {
		printf("Current host events: invalid\n");
		return -1;
	}

	printf("Current host events: 0x%08x\n", events);
	return 0;
}


int cmd_host_event_get_b(int argc, char *argv[])
{
	struct ec_response_host_event_mask r;
	int rv;

	rv = ec_command(EC_CMD_HOST_EVENT_GET_B, 0,
			NULL, 0, &r, sizeof(r));
	if (rv < 0)
		return rv;
	if (rv < sizeof(r)) {
		fprintf(stderr, "Insufficient data received.\n");
		return -1;
	}

	if (r.mask & EC_HOST_EVENT_MASK(EC_HOST_EVENT_INVALID)) {
		printf("Current host events-B: invalid\n");
		return -1;
	}

	printf("Current host events-B: 0x%08x\n", r.mask);
	return 0;
}


int cmd_host_event_get_smi_mask(int argc, char *argv[])
{
	struct ec_response_host_event_mask r;
	int rv;

	rv = ec_command(EC_CMD_HOST_EVENT_GET_SMI_MASK, 0,
			NULL, 0, &r, sizeof(r));
	if (rv < 0)
		return rv;

	printf("Current host event SMI mask: 0x%08x\n", r.mask);
	return 0;
}


int cmd_host_event_get_sci_mask(int argc, char *argv[])
{
	struct ec_response_host_event_mask r;
	int rv;

	rv = ec_command(EC_CMD_HOST_EVENT_GET_SCI_MASK, 0,
			NULL, 0, &r, sizeof(r));
	if (rv < 0)
		return rv;

	printf("Current host event SCI mask: 0x%08x\n", r.mask);
	return 0;
}


int cmd_host_event_get_wake_mask(int argc, char *argv[])
{
	struct ec_response_host_event_mask r;
	int rv;

	rv = ec_command(EC_CMD_HOST_EVENT_GET_WAKE_MASK, 0,
			NULL, 0, &r, sizeof(r));
	if (rv < 0)
		return rv;

	printf("Current host event wake mask: 0x%08x\n", r.mask);
	return 0;
}


int cmd_host_event_set_smi_mask(int argc, char *argv[])
{
	struct ec_params_host_event_mask p;
	char *e;
	int rv;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <mask>\n", argv[0]);
		return -1;
	}
	p.mask = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad mask.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_HOST_EVENT_SET_SMI_MASK, 0,
			&p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	printf("Mask set.\n");
	return 0;
}


int cmd_host_event_set_sci_mask(int argc, char *argv[])
{
	struct ec_params_host_event_mask p;
	char *e;
	int rv;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <mask>\n", argv[0]);
		return -1;
	}
	p.mask = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad mask.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_HOST_EVENT_SET_SCI_MASK, 0,
			&p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	printf("Mask set.\n");
	return 0;
}


int cmd_host_event_set_wake_mask(int argc, char *argv[])
{
	struct ec_params_host_event_mask p;
	char *e;
	int rv;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <mask>\n", argv[0]);
		return -1;
	}
	p.mask = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad mask.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_HOST_EVENT_SET_WAKE_MASK, 0,
			&p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	printf("Mask set.\n");
	return 0;
}


int cmd_host_event_clear(int argc, char *argv[])
{
	struct ec_params_host_event_mask p;
	char *e;
	int rv;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <mask>\n", argv[0]);
		return -1;
	}
	p.mask = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad mask.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_HOST_EVENT_CLEAR, 0,
			&p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	printf("Host events cleared.\n");
	return 0;
}


int cmd_host_event_clear_b(int argc, char *argv[])
{
	struct ec_params_host_event_mask p;
	char *e;
	int rv;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <mask>\n", argv[0]);
		return -1;
	}
	p.mask = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad mask.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_HOST_EVENT_CLEAR_B, 0,
			&p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	printf("Host events-B cleared.\n");
	return 0;
}


int cmd_gpio_get(int argc, char *argv[])
{
	struct ec_params_gpio_get_v1 p_v1;
	struct ec_response_gpio_get_v1 r_v1;
	int i, rv, subcmd, num_gpios;
	int cmdver = 1;

	if (!ec_cmd_version_supported(EC_CMD_GPIO_GET, cmdver)) {
		struct ec_params_gpio_get p;
		struct ec_response_gpio_get r;

		/* Fall back to version 0 command */
		cmdver = 0;
		if (argc != 2) {
			fprintf(stderr, "Usage: %s <GPIO name>\n", argv[0]);
			return -1;
		}

		if (strlen(argv[1]) + 1 > sizeof(p.name)) {
			fprintf(stderr, "GPIO name too long.\n");
			return -1;
		}
		strcpy(p.name, argv[1]);

		rv = ec_command(EC_CMD_GPIO_GET, cmdver, &p,
				sizeof(p), &r, sizeof(r));
		if (rv < 0)
			return rv;

		printf("GPIO %s = %d\n", p.name, r.val);
		return 0;
	}

	if (argc > 2 || (argc == 2 && !strcmp(argv[1], "help"))) {
		printf("Usage: %s [<subcmd> <GPIO name>]\n", argv[0]);
		printf("'gpioget <GPIO_NAME>' - Get value by name\n");
		printf("'gpioget count' - Get count of GPIOS\n");
		printf("'gpioget all' - Get info for all GPIOs\n");
		return -1;
	}

	/* Keeping it consistent with console command behavior */
	if (argc == 1)
		subcmd = EC_GPIO_GET_INFO;
	else if (!strcmp(argv[1], "count"))
		subcmd = EC_GPIO_GET_COUNT;
	else if (!strcmp(argv[1], "all"))
		subcmd = EC_GPIO_GET_INFO;
	else
		subcmd = EC_GPIO_GET_BY_NAME;

	if (subcmd == EC_GPIO_GET_BY_NAME) {
		p_v1.subcmd = EC_GPIO_GET_BY_NAME;
		if (strlen(argv[1]) + 1 > sizeof(p_v1.get_value_by_name.name)) {
			fprintf(stderr, "GPIO name too long.\n");
			return -1;
		}
		strcpy(p_v1.get_value_by_name.name, argv[1]);

		rv = ec_command(EC_CMD_GPIO_GET, cmdver, &p_v1,
				sizeof(p_v1), &r_v1, sizeof(r_v1));

		if (rv < 0)
			return rv;

		printf("GPIO %s = %d\n", p_v1.get_value_by_name.name,
			r_v1.get_value_by_name.val);
		return 0;
	}

	/* Need GPIO count for EC_GPIO_GET_COUNT or EC_GPIO_GET_INFO */
	p_v1.subcmd = EC_GPIO_GET_COUNT;
	rv = ec_command(EC_CMD_GPIO_GET, cmdver, &p_v1,
			sizeof(p_v1), &r_v1, sizeof(r_v1));
	if (rv < 0)
		return rv;

	if (subcmd == EC_GPIO_GET_COUNT) {
		printf("GPIO COUNT = %d\n", r_v1.get_count.val);
		return 0;
	}

	/* subcmd EC_GPIO_GET_INFO */
	num_gpios = r_v1.get_count.val;
	p_v1.subcmd = EC_GPIO_GET_INFO;

	for (i = 0; i < num_gpios; i++) {
		p_v1.get_info.index = i;

		rv = ec_command(EC_CMD_GPIO_GET, cmdver, &p_v1,
				sizeof(p_v1), &r_v1, sizeof(r_v1));
		if (rv < 0)
			return rv;

		printf("%2d %-32s 0x%04X\n", r_v1.get_info.val,
			r_v1.get_info.name, r_v1.get_info.flags);
	}

	return 0;
}

int cmd_gpio_set(int argc, char *argv[])
{
	struct ec_params_gpio_set p;
	char *e;
	int rv;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <GPIO name> <0 | 1>\n", argv[0]);
		return -1;
	}

	if (strlen(argv[1]) + 1 > sizeof(p.name)) {
		fprintf(stderr, "GPIO name too long.\n");
		return -1;
	}
	strcpy(p.name, argv[1]);

	p.val = strtol(argv[2], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad value.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_GPIO_SET, 0, &p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	printf("GPIO %s set to %d\n\n", p.name, p.val);
	return 0;
}


enum sysinfo_fields {
	SYSINFO_FIELD_RESET_FLAGS = BIT(0),
	SYSINFO_FIELD_CURRENT_IMAGE = BIT(1),
	SYSINFO_FIELD_FLAGS = BIT(2),
	SYSINFO_INFO_FIELD_ALL = SYSINFO_FIELD_RESET_FLAGS |
				 SYSINFO_FIELD_CURRENT_IMAGE |
				 SYSINFO_FIELD_FLAGS
};

static int sysinfo(struct ec_response_sysinfo *info)
{
	struct ec_response_sysinfo r;
	int rv;

	rv = ec_command(EC_CMD_SYSINFO, 0, NULL, 0, &r, sizeof(r));
	if (rv < 0) {
		fprintf(stderr, "ERROR: EC_CMD_SYSINFO failed: %d\n", rv);
		return rv;
	}

	return 0;
}

int cmd_sysinfo(int argc, char **argv)
{
	struct ec_response_sysinfo r;
	enum sysinfo_fields fields;
	bool print_prefix = false;

	if (argc != 1 && argc != 2)
		goto sysinfo_error_usage;

	if (argc == 1) {
		fields = SYSINFO_INFO_FIELD_ALL;
		print_prefix = true;
	} else if (argc == 2) {
		if (strcmp(argv[1], "flags") == 0)
			fields = SYSINFO_FIELD_FLAGS;
		else if (strcmp(argv[1], "reset_flags") == 0)
			fields = SYSINFO_FIELD_RESET_FLAGS;
		else if (strcmp(argv[1], "firmware_copy") == 0)
			fields = SYSINFO_FIELD_CURRENT_IMAGE;
		else
			goto sysinfo_error_usage;
	}

	if (sysinfo(&r) != 0)
		return -1;

	if (fields & SYSINFO_FIELD_RESET_FLAGS) {
		if (print_prefix)
			printf("Reset flags: ");
		printf("0x%08x\n", r.reset_flags);
	}

	if (fields & SYSINFO_FIELD_FLAGS) {
		if (print_prefix)
			printf("Flags: ");
		printf("0x%08x\n", r.flags);

	}

	if (fields & SYSINFO_FIELD_CURRENT_IMAGE) {
		if (print_prefix)
			printf("Firmware copy: ");
		printf("%d\n\n", r.current_image);
	}

	return 0;

sysinfo_error_usage:
	fprintf(stderr, "Usage: %s "
			"[flags|reset_flags|firmware_copy]\n",
		argv[0]);
	return -1;
}

int ec_flash_read(uint8_t *buf, int offset, int size)
{
	struct ec_params_flash_read p;
	int rv;
	int i;
    char build_string[256];

	/* Read data in chunks */
	for (i = 0; i < size; i += 0xF0)
    {
		p.offset = offset + i;
		p.size = MIN(size - i, 0xF0);
		rv = ec_command(EC_CMD_FLASH_READ, 0, &p,
                                sizeof(p), build_string, p.size);
		if (rv < 0) {
			fprintf(stderr, "Read error at offset %d\n", i);
			return rv;
		}
		memcpy(buf + i, build_string, p.size);
	}

	return 0;
}

int read_shutdown_wakeup_case_to_linklist(void)
{
	int i;
	uint32_t buf[2048];

	if((binaryFile = fopen("8k_shutdown_wakeup_cause.bin","r")) == NULL)
	{
		printf("8k_shutdown_wakeup_cause.bin not exist\n\n");
		return -1;
	}
	fread(buf, 4, 2048, binaryFile);
	fclose(binaryFile);
	
	for(i = 32; i < 2048; i+=2)
	{
		if(i >= 1024 && i < (1024+32) || 0xffffffff == buf[i])
			continue;
		
		//flag = 1, shutdown cause
		//flag = 0, shutdown cause
		if(i <= 1024)
			add_data_from_tail(&raw_log_head, &raw_log_tail, buf[i+1], buf[i], 1);
		else
			add_data_from_tail(&raw_log_head, &raw_log_tail, buf[i+1], buf[i], 0);
	}
	
	return 0;
}

int cmd_log_info(int argc, char *argv[])
{
	int rv;

    if (argc != 1)
    {
		fprintf(stderr, "Usage: %s \n", argv[0]);
		return -1;
	}

	rv = read_shutdown_case_into_linklist();
	if(rv < 0)
	{
		return rv;
	}

	rv = read_wakeup_case_into_linklist();
	if(rv < 0)
	{
		return rv;
	}

	txtFile = fopen("LogInfo.txt","w");
	if(txtFile == NULL)
	{
		printf("Creat default LogInfo.txt file Fail\n\n");
		return -1;
	}
	
	read_shutdown_wakeup_case_to_linklist();
	sort_raw_log_linklist(raw_log_head);
	analysis_seq_linklist(seq_log_head);
	
	fclose(txtFile);
	printf("\nCreat LogInfo.txt file OK\n\n");

	free_linklist(shutdown_case_head);
	free_linklist(wakeup_case_head);
	
	return 0;
}

int cmd_read_8k_log(int argc, char *argv[])
{
	int offset, size;
	int rv;
	char *e;
	uint8_t *buf;

	offset = 0x3c000;
	size = 0x2000;
	
	printf("Reading %d bytes at offset %d...\n", size, offset);

	buf = (uint8_t *)malloc(size);
	if (!buf) {
		fprintf(stderr, "Unable to allocate buffer.\n");
		return -1;
	}

	/* Read data in chunks */
	rv = ec_flash_read(buf, offset, size);
	if (rv < 0) {
		free(buf);
		return rv;
	}

	rv = write_file("8k_shutdown_wakeup_cause.bin", buf, size);
	free(buf);
	if (rv)
		return rv;

	printf("done.\n\n");
	return 0;
}



int cmd_analysis_log(int argc, char *argv[])
{
	int i, rv;
	char date[32];
	uint32_t buf[2048];

    /********************************************
    * shutdown cause
    ********************************************/
	rv = read_shutdown_case_into_linklist();
	if(rv < 0)
	{
		return rv;
	}
	
	if((binaryFile = fopen("8k_shutdown_wakeup_cause.bin","r")) == NULL)
	{
		printf("8k_shutdown_wakeup_cause.bin not exist\n\n");
		return -1;
	}

	fread(buf, 4, 2048, binaryFile);
    fclose(binaryFile);
    
	SDTxtFile = fopen("shutdown-cause.txt","w");
	if(SDTxtFile == NULL)
	{
		printf("Creat shutdown-cause.txt file Fail\n\n");
		return -1;
	}
	
	for (i = 32; i < 1024; i+=2)
    {
   		if(0xffffffff == buf[i])
			break;

        // date and time
    	sec_to_date(buf[i+1], date);
		analysis_data(date);
		fprintf(SDTxtFile, "[%s] : ID=[%02X], ", date, buf[i]);

        // shutdown cause
		if( analysis_log_info(shutdown_case_head, buf[i], SDTxtFile) )
		{
			fprintf(SDTxtFile, "unkonw log id\n");
		}
	}
	printf("Creat shutdown-cause.txt file OK\n\n");
    fclose(SDTxtFile);
    
    /*********************************************
    * wakeup cause
    *********************************************/
    rv = read_wakeup_case_into_linklist();
	if(rv < 0)
	{
		return rv;
	}

	WUTxtFile = fopen("wakeup-cause.txt","w");
	if(WUTxtFile == NULL)
	{
		printf("Creat wakeup-cause.txt file Fail\n\n");
		return -1;
	}

    for (i = (1024+32); i < 2048; i+=2)
    {
		if(0xffffffff == buf[i])
			break;

        // date and time
    	sec_to_date(buf[i+1], date);
		analysis_data(date);
		fprintf(WUTxtFile, "[%s] : ID=[%02X], ", date, buf[i]);

        // wakeup cause
		if( analysis_log_info(wakeup_case_head, buf[i], WUTxtFile) )
		{
			fprintf(WUTxtFile, "unkonw log id\n");
		}
	}
    fclose(WUTxtFile);
	printf("Creat wakeup-case.txt file OK\n\n");

	return 0;
}


int cmd_mfg_data_read(int argc, char *argv[])
{
	struct ec_params_mfg_data p;
	struct ec_response_mfg_data r;
	char *e;
	int rv;
	
	if (argc != 2) {
		fprintf(stderr,"Usage: %s <index>\n", argv[0]);
		return -1;
	}

	p.index = strtol(argv[1], &e, 0);
	if (e && *e)
    {
		fprintf(stderr, "Bad index.\n");
		return -1;
	}
	
	rv = ec_command(EC_CMD_FLASH_GET_MFG_DATA, 0, &p, sizeof(p), &r, sizeof(r));
	if (rv < 0)
		return rv;
	
	printf("index 0x%02x data = 0x%02x\n", p.index, r.data);
	return 0;
}

int cmd_mfg_data_write(int argc, char *argv[])
{
	struct ec_params_mfg_data p;
	struct ec_response_mfg_data r;
	char *e;
	int rv;
	
	if (argc != 3) {
		fprintf(stderr,"Usage: %s <index> <data>\n", argv[0]);
		return -1;
	}

	p.index = strtol(argv[1], &e, 0);
	if (e && *e)
	{
		fprintf(stderr, "Bad index.\n");
		return -1;
	}
	
	p.data = strtol(argv[2], &e, 0);
	if (e && *e)
	{
		fprintf(stderr, "Bad data.\n");
		return -1;
	}
	
	rv = ec_command(EC_CMD_FLASH_SET_MFG_DATA, 0, &p, sizeof(p), &r, sizeof(r));
	if (rv < 0)
		return rv;
	
	printf("write 0x%02x in 0x%02x success!\n", p.data, p.index);
	return 0;	
}


int cmd_reboot_ec(int argc, char *argv[])
{
	struct ec_params_reboot_ec p;
	int rv, i;

	if (argc < 2) {
		/*
		 * No params specified so tell the EC to reboot immediately.
		 * That reboots the AP as well, so unlikely we'll be around
		 * to see a return code from this...
		 */
		rv = ec_command(EC_CMD_REBOOT, 0, NULL, 0, NULL, 0);
		return (rv < 0 ? rv : 0);
	}

	/* Parse command */
	if (!strcmp(argv[1], "cancel"))
		p.cmd = EC_REBOOT_CANCEL;
	else if (!strcmp(argv[1], "RO"))
		p.cmd = EC_REBOOT_JUMP_RO;
	else if (!strcmp(argv[1], "RW"))
		p.cmd = EC_REBOOT_JUMP_RW;
	else if (!strcmp(argv[1], "cold"))
		p.cmd = EC_REBOOT_COLD;
	else if (!strcmp(argv[1], "disable-jump"))
		p.cmd = EC_REBOOT_DISABLE_JUMP;
	else if (!strcmp(argv[1], "hibernate"))
		p.cmd = EC_REBOOT_HIBERNATE;
	else if (!strcmp(argv[1], "hibernate-clear-ap-off"))
		p.cmd = EC_REBOOT_HIBERNATE_CLEAR_AP_OFF;
	else if (!strcmp(argv[1], "cold-ap-off"))
			p.cmd = EC_REBOOT_COLD_AP_OFF;
	else {
		fprintf(stderr, "Unknown command: %s\n", argv[1]);
		return -1;
	}

	/* Parse flags, if any */
	p.flags = 0;
	for (i = 2; i < argc; i++) {
		if (!strcmp(argv[i], "at-shutdown")) {
			p.flags |= EC_REBOOT_FLAG_ON_AP_SHUTDOWN;
		} else if (!strcmp(argv[i], "switch-slot")) {
			p.flags |= EC_REBOOT_FLAG_SWITCH_RW_SLOT;
		} else {
			fprintf(stderr, "Unknown flag: %s\n", argv[i]);
			return -1;
		}
	}

	rv = ec_command(EC_CMD_REBOOT_EC, 0, &p, sizeof(p), NULL, 0);
	return (rv < 0 ? rv : 0);
}

int cmd_rtc_get(int argc, char *argv[])
{
	struct ec_response_rtc r;
	int rv;

	rv = ec_command(EC_CMD_RTC_GET_VALUE, 0, NULL, 0, &r, sizeof(r));
	if (rv < 0)
		return rv;

	printf("Current time: 0x%08x (%d)\n", r.time, r.time);
	return 0;
}


int cmd_rtc_set(int argc, char *argv[])
{
	struct ec_params_rtc p;
	char *e;
	int rv;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <time>\n", argv[0]);
		return -1;
	}
	p.time = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad time.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_RTC_SET_VALUE, 0, &p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	printf("Time set.\n");
	return 0;
}

int cmd_rtc_set_alarm(int argc, char *argv[])
{
	struct ec_params_rtc p;
	char *e;
	int rv;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <sec>\n", argv[0]);
		return -1;
	}
	p.time = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad time.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_RTC_SET_ALARM, 0, &p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	if (p.time == 0)
		printf("Disabling alarm.\n");
	else
		printf("Alarm set to go off in %d secs.\n", p.time);
	return 0;
}

int cmd_rtc_get_alarm(int argc, char *argv[])
{
	struct ec_response_rtc r;
	int rv;

	rv = ec_command(EC_CMD_RTC_GET_ALARM, 0, NULL, 0, &r, sizeof(r));
	if (rv < 0)
		return rv;

	if (r.time == 0)
		printf("Alarm not set\n");
	else
		printf("Alarm to go off in %d secs\n", r.time);
	return 0;
}

int ec_flash_verify(const uint8_t *buf, int offset, int size)
{
	uint8_t *rbuf = (uint8_t *)malloc(size);
	int rv;
	int i;

	if (!rbuf) {
		fprintf(stderr, "Unable to allocate buffer.\n");
		return -1;
	}

	rv = ec_flash_read(rbuf, offset, size);
	if (rv < 0) {
		free(rbuf);
		return rv;
	}

	for (i = 0; i < size; i++) {
		if (buf[i] != rbuf[i]) {
			fprintf(stderr, "Mismatch at offset 0x%x: "
				"want 0x%02x, got 0x%02x\n",
				i, buf[i], rbuf[i]);
			free(rbuf);
			return -1;
		}
	}

	free(rbuf);
	return 0;
}

/* Maximum flash size (16 MB, conservative) */
#define MAX_FLASH_SIZE 0x1000000

int cmd_flash_read(int argc, char *argv[])
{
	int offset, size;
	int rv;
	char *e;
	uint8_t *buf;

	if (argc < 4) {
		fprintf(stderr,
			"Usage: %s <offset> <size> <filename>\n", argv[0]);
		return -1;
	}
	offset = strtol(argv[1], &e, 0);
	if ((e && *e) || offset < 0 || offset > MAX_FLASH_SIZE) {
		fprintf(stderr, "Bad offset.\n");
		return -1;
	}
	size = strtol(argv[2], &e, 0);
	if ((e && *e) || size <= 0 || size > MAX_FLASH_SIZE) {
		fprintf(stderr, "Bad size.\n");
		return -1;
	}
	printf("Reading %d bytes at offset %d...\n", size, offset);

	buf = (uint8_t *)malloc(size);
	if (!buf) {
		fprintf(stderr, "Unable to allocate buffer.\n");
		return -1;
	}

	/* Read data in chunks */
	rv = ec_flash_read(buf, offset, size);
	if (rv < 0) {
		free(buf);
		return rv;
	}

	rv = write_file(argv[3], buf, size);
	free(buf);
	if (rv)
		return rv;

	printf("done.\n\n");
	return 0;
}

int cmd_backup_ec(int argc, char *argv[])
{
    struct ec_params_flash_read p;
    int offset, size;
	int rv;
	char *e;
	uint8_t *buf;
    int i;
    char build_string[256];

    if (argc < 2)
    {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -1;
	}

    offset = 0;     // NPCX796FC flash start address
    size = 0x80000; // NPCX796FC flash size
	printf("\nReading NPCX796FC internal flash %dK bytes at offset %d...\n", 
                            size/1024, offset);

	buf = (uint8_t *)malloc(size);
	if (!buf)
    {
		fprintf(stderr, "Unable to allocate buffer.\n");
		return -1;
	}

	/* Read data in chunks */
	for (i = 0; i < size; i += 0xF0)
    {
		p.offset = offset + i;
		p.size = MIN(size - i, 0xF0);

		rv = ec_command(EC_CMD_FLASH_READ, 0, &p,
                                sizeof(p), build_string, p.size);
		if (rv < 0)
        {
            free(buf);
			fprintf(stderr, "Read error at offset %d\n", i);
			return rv;
		}
		memcpy(buf + i, build_string, p.size);

        if(!(i%0x888))
        {
            printf("#%3d%%\b\b\b\b",(i*100)/0x80000);
        }
	}
    printf("%3d%%",100);

	rv = write_file(argv[1], buf, size);
	free(buf);
	if (rv)
		return rv;

	printf("\ndone.\n\n");
	return 0;
}

int cmd_flash_ec(int argc, char *argv[])
{
    char build_string[256];
	struct ec_params_flash_write *p =
                        (struct ec_params_flash_write *)build_string;
	int i;
	int offset, size;
	int rv;
	char *e;
	uint8_t *buf;
    struct ec_params_flash_erase p_erase;

	if (argc < 2)
    {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -1;
	}

	/* Read the input file */
	buf = (uint8_t *)read_file(argv[1], &size);
	if (!buf)
		return -1;

    if(size > 0x80000)
    {
        fprintf(stderr, "Binary file size too big!\n\n", argv[0]);
		return -1;
    }

    /* Erase flash */
    offset = 0;     // NPCX796FC flash start address
    p_erase.offset = 0;
	p_erase.size = size;
    printf("\nErasing NPCX796FC internal flash %dK bytes at offset %d...\n\n",
                            size/1024, offset);
	rv = ec_command(EC_CMD_FLASH_ERASE, 0, &p_erase, sizeof(p_erase), NULL, 0);
    if (rv < 0)
    {
        printf("Erase Fail\n");
		return rv;
    }

    /* Write data in chunks */
    printf("\nWriting NPCX796FC internal flash %dK bytes at offset %d...\n", 
                            size/1024, offset);

	for (i = 0; i < size; i += 0xF0)
    {
		p->offset = offset + i;
		p->size = MIN(size - i, 0xF0);
		memcpy(p + 1, buf + i, p->size);
        rv = ec_command(EC_CMD_FLASH_WRITE, 0, p, sizeof(*p) + p->size, NULL, 0);
		if (rv < 0)
        {
            free(buf);
			fprintf(stderr, "Write error at offset %d\n", i);
			return rv;
		}
        if(!(i%0x888))
        {
            printf("#%3d%%\b\b\b\b",(i*100)/0x80000);
        }
	}
    printf("%3d%%",100);

	printf("\ndone.\n\n");
	return 0;
}

int cmd_flash_info(int argc, char *argv[])
{
	struct ec_response_flash_info_1 r;
	int cmdver = 1;
	int rsize = sizeof(r);
	int rv;

	memset(&r, 0, sizeof(r));

	if (!ec_cmd_version_supported(EC_CMD_FLASH_INFO, cmdver))
    {
		/* Fall back to version 0 command */
		cmdver = 0;
		rsize = sizeof(struct ec_response_flash_info);
	}

	rv = ec_command(EC_CMD_FLASH_INFO, cmdver, NULL, 0, &r, rsize);
	if (rv < 0)
		return rv;

    printf("\n");
    printf(" FlashSize      : %d\n", r.flash_size);
    printf(" WriteSize      : %d\n", r.write_block_size);
    printf(" EraseSize      : %d\n", r.erase_block_size);
    printf(" ProtectSize    : %d\n", r.protect_block_size);

	if (cmdver >= 1)
    {
		/* Fields added in ver.1 available */
	    printf(" WriteIdealSize : %d\n", r.write_ideal_size);
        printf(" Flags          : 0x%x\n", r.flags);
	}
    printf("\n");

	return 0;
}

int cmd_write_log(int argc, char *argv[])
{
	struct ec_params_flash_log p;
	struct ec_response_flash_log r;
	int i, rv;
	char *e;
	char date[32];

	if(argc < 2) {
		fprintf(stderr, "Usage: %s <log_id>\n", argv[0]);
		return -1;
	}

	for(i = 1; i < argc; i++)
	{
		p.log_id = strtol(argv[i], &e, 0);
		if (e && *e) {
			fprintf(stderr, "Bad log_id arg.\n");
			return -1;
		}
		
		rv = ec_command(EC_CMD_FLASH_LOG_SET_VALUE, 0, &p, sizeof(p), &r, sizeof(r));
		if (rv < 0)
			return rv;
		
		sec_to_date(r.log_timestamp, date);
		printf("[ %s ] : ", date);
		printf("%02x\n", r.log_id);
	}
	printf("Write succeeded!\n");
	return 0;
}

int cmd_hello(int argc, char *argv[])
{
	struct ec_params_hello p;
	struct ec_response_hello r;
	int rv;

	p.in_data = 0xa0b0c0d0;

	rv = ec_command(EC_CMD_HELLO, 0, &p, sizeof(p), &r, sizeof(r));
	if (rv < 0)
		return rv;

	if (r.out_data != 0xa1b2c3d4) {
		fprintf(stderr, "Expected response 0x%08x, got 0x%08x\n",
			0xa1b2c3d4, r.out_data);
		return -1;
	}

	printf("\n Bitland Chrome EC says hello!\n\n");
	return 0;
}

static int find_led_color_by_name(const char *color)
{
	int i;

	for (i = 0; i < EC_LED_COLOR_COUNT; ++i)
		if (!strcmp(color, led_color_names[i]))
			return i;

	return -1;
}

static int find_led_id_by_name(const char *led)
{
	int i;

	for (i = 0; i < EC_LED_ID_COUNT; ++i)
		if (!strcmp(led, led_names[i]))
			return i;

	return -1;
}

int cmd_led(int argc, char *argv[])
{
	struct ec_params_led_control p;
	struct ec_response_led_control r;
	char *e, *ptr;
	int rv, i, j;

	memset(p.brightness, 0, sizeof(p.brightness));
	p.flags = 0;

	if (argc < 3) {
		fprintf(stderr,
			"Usage: %s <name> <query | auto | "
			"off | <color> | <color>=<value>...>\n", argv[0]);
		return -1;
	}

	p.led_id = find_led_id_by_name(argv[1]);
	if (p.led_id == (uint8_t)-1) {
		fprintf(stderr, "Bad LED name: %s\n", argv[1]);
		fprintf(stderr, "Valid LED names: ");
		for (i = 0; i < EC_LED_ID_COUNT; i++)
			fprintf(stderr, "%s ", led_names[i]);
		fprintf(stderr, "\n");
		return -1;
	}

	if (!strcmp(argv[2], "query")) {
		p.flags = EC_LED_FLAGS_QUERY;
		rv = ec_command(EC_CMD_LED_CONTROL, 1, &p, sizeof(p),
				&r, sizeof(r));
		printf("Brightness range for LED %d:\n", p.led_id);
		if (rv < 0) {
			fprintf(stderr, "Error: Unsupported LED.\n");
			return rv;
		}
		for (i = 0; i < EC_LED_COLOR_COUNT; ++i)
			printf("\t%s\t: 0x%x\n",
			       led_color_names[i],
			       r.brightness_range[i]);
		return 0;
	}

	if (!strcmp(argv[2], "off")) {
		/* Brightness initialized to 0 for each color. */
	} else if (!strcmp(argv[2], "auto")) {
		p.flags = EC_LED_FLAGS_AUTO;
	} else if ((i = find_led_color_by_name(argv[2])) != -1) {
		p.brightness[i] = 0xff;
	} else {
		for (i = 2; i < argc; ++i) {
			ptr = strtok(argv[i], "=");
			j = find_led_color_by_name(ptr);
			if (j == -1) {
				fprintf(stderr, "Bad color name: %s\n", ptr);
				fprintf(stderr, "Valid colors: ");
				for (j = 0; j < EC_LED_COLOR_COUNT; j++)
					fprintf(stderr, "%s ",
						led_color_names[j]);
				fprintf(stderr, "\n");
				return -1;
			}
			ptr = strtok(NULL, "=");
			if (ptr == NULL) {
				fprintf(stderr, "Missing brightness value\n");
				return -1;
			}
			p.brightness[j] = strtol(ptr, &e, 0);
			if (e && *e) {
				fprintf(stderr, "Bad brightness: %s\n", ptr);
				return -1;
			}
		}
	}

	rv = ec_command(EC_CMD_LED_CONTROL, 1, &p, sizeof(p), &r, sizeof(r));
	return (rv < 0 ? rv : 0);
}

int cmd_pwrbtn_test_start(int argc, char *argv[])
{
	int rv;
	rv = ec_command(EC_CMD_POWER_BUTTON_TEST_START, 0, NULL, 0, NULL, 0);
	if(rv < 0) {
		printf("rv = %d\n", rv);
		return -1;
	}
	return 0;
}

int cmd_pwrbtn_test_end(int argc, char *argv[])
{
	struct ec_response_test_start r;
	int rv;
	
	rv = ec_command(EC_CMD_POWER_BUTTON_TEST_END, 0, NULL, 0,  &r, sizeof(r));
	if(rv < 0 ) {
		printf("rv = %d\n", rv);
		return -1;
	}

	printf("power button rising count : %d\n", r.rising);
	printf("power button falling count : %d\n", r.falling);
	
	return 0;
}

int cmd_power_info(int argc, char *argv[])
{
	struct ec_response_power_info_v1 r;
	int rv;

	rv = ec_command(EC_CMD_POWER_INFO, 1, NULL, 0, &r, sizeof(r));
	if (rv < 0)
		return rv;

	printf("Power source:\t");
	switch (r.system_power_source) {
	case POWER_SOURCE_UNKNOWN:
		printf("Unknown\n");
		break;
	case POWER_SOURCE_BATTERY:
		printf("Battery\n");
		break;
	case POWER_SOURCE_AC:
		printf("AC\n");
		break;
	case POWER_SOURCE_AC_BATTERY:
		printf("AC + battery\n");
		break;
	}

	printf("Battery state-of-charge: %d%%\n", r.battery_soc);
	printf("Max AC power: %d Watts\n", r.ac_adapter_100pct);
	printf("Battery 1Cd rate: %d\n", r.battery_1cd);
	printf("RoP Avg: %d Watts\n", r.rop_avg);
	printf("RoP Peak: %d Watts\n", r.rop_peak);
	printf("Battery DBPT support level: %d\n",
	       r.intel.batt_dbpt_support_level);
	printf("Battery DBPT Max Peak Power: %d Watts\n",
	       r.intel.batt_dbpt_max_peak_power);
	printf("Battery DBPT Sus Peak Power: %d Watts\n",
	       r.intel.batt_dbpt_sus_peak_power);
	return 0;
}

int read_mapped_temperature(int id)
{
	int rv;
	if (id < EC_TEMP_SENSOR_ENTRIES)
	{
		rv = read_mapped_mem8(EC_MEMMAP_TEMP_SENSOR + id);
	}
	else 
	{
		/* Sensor in second bank, but second bank isn't supported */
		rv = EC_TEMP_SENSOR_NOT_PRESENT;
	}

	return rv;


	#if 0
	int rv;

	if (!read_mapped_mem8(EC_MEMMAP_THERMAL_VERSION)) {
		/*
		 *  The temp_sensor_init() is not called, which implies no
		 * temp sensor is defined.
		 */
		rv = EC_TEMP_SENSOR_NOT_PRESENT;
	} else if (id < EC_TEMP_SENSOR_ENTRIES)
		rv = read_mapped_mem8(EC_MEMMAP_TEMP_SENSOR + id);
	else if (read_mapped_mem8(EC_MEMMAP_THERMAL_VERSION) >= 2)
		rv = read_mapped_mem8(EC_MEMMAP_TEMP_SENSOR_B +
				      id - EC_TEMP_SENSOR_ENTRIES);
	else {
		/* Sensor in second bank, but second bank isn't supported */
		rv = EC_TEMP_SENSOR_NOT_PRESENT;
	}
	return rv;
	#endif
}

int cmd_temperature(int argc, char *argv[])
{
	int rv;
	int id;
	char *e;

	if (argc != 2)
    {
		fprintf(stderr, " Usage: %s <sensorid> | all\n\n", argv[0]);
		return -1;
	}

	if (strcmp(argv[1], "all") == 0)
    {
        printf(" Thermal Sensor list :\n");
		for (id = 0;
		     id < EC_TEMP_SENSOR_ENTRIES;
		     id++) {
			rv = read_mapped_temperature(id);
			switch (rv) {
			case EC_TEMP_SENSOR_NOT_PRESENT:
				break;
			case EC_TEMP_SENSOR_ERROR:
				fprintf(stderr, "Sensor %d error\n", id);
				break;
			case EC_TEMP_SENSOR_NOT_POWERED:
				fprintf(stderr, "Sensor %d disabled\n", id);
				break;
			case EC_TEMP_SENSOR_NOT_CALIBRATED:
				fprintf(stderr, "Sensor %d not calibrated\n",
					id);
				break;
			default:
				printf(" Sensor ID[%d]: %d(K) %.1f(C)\n",
                            id, rv + EC_TEMP_SENSOR_OFFSET, (rv-73.15));
			}
		}
		return 0;
	}

	id = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad sensor ID.\n");
		return -1;
	}

	if (id < 0 ||
	    id >= EC_TEMP_SENSOR_ENTRIES) {
		printf("Sensor ID invalid.\n");
		return -1;
	}

	printf("Reading temperature...\n");
	rv = read_mapped_temperature(id);

	switch (rv) {
	case EC_TEMP_SENSOR_NOT_PRESENT:
		printf("Sensor not present\n");
		return -1;
	case EC_TEMP_SENSOR_ERROR:
		printf("Error\n");
		return -1;
	case EC_TEMP_SENSOR_NOT_POWERED:
		printf("Sensor disabled/unpowered\n");
		return -1;
	case EC_TEMP_SENSOR_NOT_CALIBRATED:
		fprintf(stderr, "Sensor not calibrated\n");
		return -1;
	default:
		printf(" Sensor ID[%d]: %d(K) %.1f(C)\n",
                            id, rv + EC_TEMP_SENSOR_OFFSET, (rv-73.15));
		return 0;
	}
}

int cmd_temp_sensor_info(int argc, char *argv[])
{
	struct ec_params_temp_sensor_get_info p;
	struct ec_response_temp_sensor_get_info r;
	int rv;
	char *e;

	if (argc != 2)
    {
		fprintf(stderr, " Usage: %s <sensorid> | all\n\n", argv[0]);
		return -1;
	}

	if (strcmp(argv[1], "all") == 0)
    {
        printf(" Thermal Sensor list :\n");
		for (p.id = 0;
		     p.id < EC_TEMP_SENSOR_ENTRIES;
		     p.id++) {
			if (read_mapped_temperature(p.id) ==
			    EC_TEMP_SENSOR_NOT_PRESENT)
				continue;
			rv = ec_command(EC_CMD_TEMP_SENSOR_GET_INFO, 0,
					&p, sizeof(p), &r, sizeof(r));
			if (rv < 0)
				continue;
			printf(" Sensor ID[%d]: Type=%d Name=%s\n", p.id, r.sensor_type,
			       r.sensor_name);
		}
		return 0;
	}

	p.id = strtol(argv[1], &e, 0);
	if (e && *e)
    {
		fprintf(stderr, "Bad sensor ID.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_TEMP_SENSOR_GET_INFO, 0,
			&p, sizeof(p), &r, sizeof(r));
	if (rv < 0)
		return rv;

	printf(" Sensor name: %s\n", r.sensor_name);
	printf(" Sensor type: %d\n", r.sensor_type);

	return 0;
}

int cmd_thermal_get_threshold_v0(int argc, char *argv[])
{
	struct ec_params_thermal_get_threshold p;
	struct ec_response_thermal_get_threshold r;
	char *e;
	int rv;

	if (argc != 3) {
		fprintf(stderr,
			"Usage: %s <sensortypeid> <thresholdid>\n", argv[0]);
		return -1;
	}

	p.sensor_type = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad sensor type ID.\n");
		return -1;
	}

	p.threshold_id = strtol(argv[2], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad threshold ID.\n");
		return -1;
	}

	rv = ec_command(EC_CMD_THERMAL_GET_THRESHOLD, 0,
			&p, sizeof(p), &r, sizeof(r));
	if (rv < 0)
		return rv;

	printf("Threshold %d for sensor type %d is %d K.\n",
			p.threshold_id, p.sensor_type, r.value);

	return 0;
}

int cmd_thermal_get_threshold_v1(int argc, char *argv[])
{
	struct ec_params_thermal_get_threshold_v1 p;
	struct ec_thermal_config r;
	struct ec_params_temp_sensor_get_info pi;
	struct ec_response_temp_sensor_get_info ri;
	int rv;
	int i;

	printf("sensor  warn  high  halt   fan_off fan_max   name\n");
	for (i = 0; i < 99; i++) {	/* number of sensors is unknown */

		/* ask for one */
		p.sensor_num = i;
		rv = ec_command(EC_CMD_THERMAL_GET_THRESHOLD, 1,
				&p, sizeof(p), &r, sizeof(r));
		if (rv <= 0)		/* stop on first failure */
			break;

		/* ask for its name, too */
		pi.id = i;
		rv = ec_command(EC_CMD_TEMP_SENSOR_GET_INFO, 0,
				&pi, sizeof(pi), &ri, sizeof(ri));

		/* print what we know */
		printf(" %2d      %3d   %3d    %3d    %3d     %3d     %s\n",
		       i,
		       r.temp_host[EC_TEMP_THRESH_WARN],
		       r.temp_host[EC_TEMP_THRESH_HIGH],
		       r.temp_host[EC_TEMP_THRESH_HALT],
		       r.temp_fan_off, r.temp_fan_max,
		       rv > 0 ? ri.sensor_name : "?");
	}
	if (i)
		printf("(all temps in degrees Kelvin)\n");

	return 0;
}

int cmd_thermal_get_threshold(int argc, char *argv[])
{
	if (ec_cmd_version_supported(EC_CMD_THERMAL_GET_THRESHOLD, 1))
		return cmd_thermal_get_threshold_v1(argc, argv);
	else if (ec_cmd_version_supported(EC_CMD_THERMAL_GET_THRESHOLD, 0))
		return cmd_thermal_get_threshold_v0(argc, argv);

	printf("I got nuthin.\n");
	return -1;
}

/* Print shared fields of sink and source cap PDOs */
static inline void print_pdo_fixed(uint32_t pdo)
{
	printf("    Fixed: %dmV %dmA %s%s%s%s",
	       PDO_FIXED_VOLTAGE(pdo),
	       PDO_FIXED_CURRENT(pdo),
	       pdo & PDO_FIXED_DUAL_ROLE ? "DRP " : "",
	       pdo & PDO_FIXED_UNCONSTRAINED ? "UP " : "",
	       pdo & PDO_FIXED_COMM_CAP ? "USB " : "",
	       pdo & PDO_FIXED_DATA_SWAP ? "DRD" : "");
}

static inline void print_pdo_battery(uint32_t pdo)
{
	printf("    Battery: max %dmV min %dmV max %dmW\n",
	       PDO_BATT_MAX_VOLTAGE(pdo),
	       PDO_BATT_MIN_VOLTAGE(pdo),
	       PDO_BATT_MAX_POWER(pdo));
}

static inline void print_pdo_variable(uint32_t pdo)
{
	printf("    Variable: max %dmV min %dmV max %dmA\n",
	       PDO_VAR_MAX_VOLTAGE(pdo),
	       PDO_VAR_MIN_VOLTAGE(pdo),
	       PDO_VAR_MAX_CURRENT(pdo));
}

static inline void print_pdo_augmented(uint32_t pdo)
{
	printf("    Augmented: max %dmV min %dmV max %dmA\n",
	       PDO_AUG_MAX_VOLTAGE(pdo),
	       PDO_AUG_MIN_VOLTAGE(pdo),
	       PDO_AUG_MAX_CURRENT(pdo));
}

int cmd_usb_pd(int argc, char *argv[])
{
	const char *role_str[] = {"", "toggle", "toggle-off", "sink", "source",
				  "freeze"};
	const char *mux_str[] = {"", "none", "usb", "dp", "dock", "auto"};
	const char *swap_str[] = {"", "dr_swap", "pr_swap", "vconn_swap"};
	struct ec_params_usb_pd_control p;
	struct ec_response_usb_pd_control_v2 *r_v2 =
		(struct ec_response_usb_pd_control_v2 *)ec_inbuf;
	struct ec_response_usb_pd_control_v1 *r_v1 =
		(struct ec_response_usb_pd_control_v1 *)ec_inbuf;
	struct ec_response_usb_pd_control *r =
		(struct ec_response_usb_pd_control *)ec_inbuf;
	int rv, i, j;
	int option_ok;
	char *e;
	int cmdver;

	BUILD_ASSERT(ARRAY_SIZE(role_str) == USB_PD_CTRL_ROLE_COUNT);
	BUILD_ASSERT(ARRAY_SIZE(mux_str) == USB_PD_CTRL_MUX_COUNT);
	BUILD_ASSERT(ARRAY_SIZE(swap_str) == USB_PD_CTRL_SWAP_COUNT);
	p.role = USB_PD_CTRL_ROLE_NO_CHANGE;
	p.mux = USB_PD_CTRL_MUX_NO_CHANGE;
	p.swap = USB_PD_CTRL_SWAP_NONE;

	if (argc < 2) {
		fprintf(stderr, "No port specified.\n");
		return -1;
	}

	p.port = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Invalid param (port)\n");
		return -1;
	}

	for (i = 2; i < argc; ++i) {
		option_ok = 0;
		if (!strcmp(argv[i], "auto")) {
			if (argc != 3) {
				fprintf(stderr, "\"auto\" may not be used "
						"with other options.\n");
				return -1;
			}
			p.role = USB_PD_CTRL_ROLE_TOGGLE_ON;
			p.mux = USB_PD_CTRL_MUX_AUTO;
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(role_str); ++j) {
			if (!strcmp(argv[i], role_str[j])) {
				if (p.role != USB_PD_CTRL_ROLE_NO_CHANGE) {
					fprintf(stderr,
						"Only one role allowed.\n");
					return -1;
				}
				p.role = j;
				option_ok = 1;
				break;
			}
		}
		if (option_ok)
			continue;

		for (j = 0; j < ARRAY_SIZE(mux_str); ++j) {
			if (!strcmp(argv[i], mux_str[j])) {
				if (p.mux != USB_PD_CTRL_MUX_NO_CHANGE) {
					fprintf(stderr,
						"Only one mux type allowed.\n");
					return -1;
				}
				p.mux = j;
				option_ok = 1;
				break;
			}
		}
		if (option_ok)
			continue;

		for (j = 0; j < ARRAY_SIZE(swap_str); ++j) {
			if (!strcmp(argv[i], swap_str[j])) {
				if (p.swap != USB_PD_CTRL_SWAP_NONE) {
					fprintf(stderr,
						"Only one swap type allowed.\n");
					return -1;
				}
				p.swap = j;
				option_ok = 1;
				break;
			}
		}


		if (!option_ok) {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			return -1;
		}
	}

	if (ec_cmd_version_supported(EC_CMD_USB_PD_CONTROL, 2))
		cmdver = 2;
	else if (ec_cmd_version_supported(EC_CMD_USB_PD_CONTROL, 1))
		cmdver = 1;
	else
		cmdver = 0;

	rv = ec_command(EC_CMD_USB_PD_CONTROL, cmdver, &p, sizeof(p),
			ec_inbuf, ec_max_insize);

	if (rv < 0 || argc != 2)
		return (rv < 0) ? rv : 0;

	if (cmdver == 0) {
		printf("Port C%d is %sabled, Role:%s Polarity:CC%d State:%d\n",
		       p.port, (r->enabled) ? "en" : "dis",
		       r->role == PD_ROLE_SOURCE ? "SRC" : "SNK",
		       r->polarity + 1, r->state);
	} else {
		printf("Port C%d: %s, %s  State:%s\n"
		       "Role:%s %s%s, Polarity:CC%d\n",
		       p.port,
		       (r_v1->enabled & PD_CTRL_RESP_ENABLED_COMMS) ?
				"enabled" : "disabled",
		       (r_v1->enabled & PD_CTRL_RESP_ENABLED_CONNECTED) ?
				"connected" : "disconnected",
		       r_v1->state,

		       (r_v1->role & PD_CTRL_RESP_ROLE_POWER) ? "SRC" : "SNK",
		       (r_v1->role & PD_CTRL_RESP_ROLE_DATA) ? "DFP" : "UFP",
		       (r_v1->role & PD_CTRL_RESP_ROLE_VCONN) ? " VCONN" : "",
		       r_v1->polarity + 1);

		if (cmdver == 2) {
			printf("CC State:");
			if (r_v2->cc_state == PD_CC_NONE)
				printf("None");
			else if (r_v2->cc_state == PD_CC_UFP_AUDIO_ACC)
				printf("UFP Audio accessory");
			else if (r_v2->cc_state == PD_CC_UFP_DEBUG_ACC)
				printf("UFP Debug accessory");
			else if (r_v2->cc_state == PD_CC_UFP_ATTACHED)
				printf("UFP attached");
			else if (r_v2->cc_state == PD_CC_DFP_DEBUG_ACC)
				printf("DFP Debug accessory");
			else if (r_v2->cc_state == PD_CC_DFP_ATTACHED)
				printf("DFP attached");
			else
				printf("UNKNOWN");
			printf("\n");

			if (r_v2->dp_mode) {
				printf("DP pin mode:");
				if (r_v2->dp_mode == MODE_DP_PIN_A)
					printf("A");
				else if (r_v2->dp_mode == MODE_DP_PIN_B)
					printf("B");
				else if (r_v2->dp_mode == MODE_DP_PIN_C)
					printf("C");
				else if (r_v2->dp_mode == MODE_DP_PIN_D)
					printf("D");
				else if (r_v2->dp_mode == MODE_DP_PIN_E)
					printf("E");
				else if (r_v2->dp_mode == MODE_DP_PIN_F)
					printf("F");
				else
					printf("UNKNOWN");
				printf("\n");
			}

			printf("Cable type:%s\n",
				r_v2->control_flags & USB_PD_CTRL_ACTIVE_CABLE ?
					"Active" : "Passive");

			printf("TBT Adapter type:%s\n",
				r_v2->control_flags &
				USB_PD_CTRL_TBT_LEGACY_ADAPTER ?
					"Legacy" : "Gen3");

			printf("Optical Cable:%s\n",
				r_v2->control_flags &
				USB_PD_CTRL_OPTICAL_CABLE ? "True" : "False");

			printf("Link LSRX Communication:%s-directional\n",
				r_v2->control_flags &
				USB_PD_CTRL_ACTIVE_LINK_UNIDIR ? "Uni" : "Bi");

			printf("TBT Cable Speed:");
			switch (r_v2->cable_speed) {
			case TBT_SS_U31_GEN1:
				printf("TBT Gen1");
				break;
			case TBT_SS_U32_GEN1_GEN2:
				printf("TBT Gen1 and TBT Gen2");
				break;
			case TBT_SS_TBT_GEN3:
				printf("TBT Gen3");
				break;
			default:
				printf("UNKNOWN");
			}
			printf("\n");

			printf("Rounded support: 3rd Gen %srounded support\n",
				r_v2->cable_gen ? "and 4th Gen " : "");
		}
		/* If connected to a PD device, then print port partner info */
		if ((r_v1->enabled & PD_CTRL_RESP_ENABLED_CONNECTED) &&
		    (r_v1->enabled & PD_CTRL_RESP_ENABLED_PD_CAPABLE))
			printf("PD Partner Capabilities:\n%s%s%s%s",
				(r_v1->role & PD_CTRL_RESP_ROLE_DR_POWER) ?
					" DR power\n" : "",
				(r_v1->role & PD_CTRL_RESP_ROLE_DR_DATA) ?
					" DR data\n" : "",
				(r_v1->role & PD_CTRL_RESP_ROLE_USB_COMM) ?
					" USB capable\n" : "",
				(r_v1->role & PD_CTRL_RESP_ROLE_UNCONSTRAINED) ?
					" Unconstrained power\n" : "");
	}
	return 0;
}


int cmd_typec_status(int argc, char *argv[])
{
	struct ec_params_typec_status p;
	struct ec_response_typec_status *r =
				(struct ec_response_typec_status *)ec_inbuf;
	char *endptr;
	int rv, i;
	char *desc;

	if (argc != 2) {
		fprintf(stderr,
			"Usage: %s <port>\n"
			"  <port> is the type-c port to query\n", argv[0]);
		return -1;
	}

	p.port = strtol(argv[1], &endptr, 0);
	if (endptr && *endptr) {
		fprintf(stderr, "Bad port\n");
		return -1;
	}

	rv = ec_command(EC_CMD_TYPEC_STATUS, 0, &p, sizeof(p),
			ec_inbuf, ec_max_insize);
	if (rv == -EC_RES_INVALID_COMMAND - EECRESULT)
		/* Fall back to PD_CONTROL to support older ECs */
		return cmd_usb_pd(argc, argv);
	else if (rv < 0)
		return -1;

	printf("Port C%d: %s, %s  State:%s\n"
	       "Role:%s %s%s, Polarity:CC%d\n",
		p.port,
		r->pd_enabled ? "enabled" : "disabled",
		r->dev_connected ? "connected" : "disconnected",
		r->tc_state,
		(r->power_role == PD_ROLE_SOURCE) ? "SRC" : "SNK",
		(r->data_role == PD_ROLE_DFP) ? "DFP" :
			(r->data_role == PD_ROLE_UFP) ? "UFP" : "",
		(r->vconn_role == PD_ROLE_VCONN_SRC) ? " VCONN" : "",
		(r->polarity % 2 + 1));

	switch (r->cc_state) {
	case PD_CC_NONE:
		desc = "None";
		break;
	case PD_CC_UFP_AUDIO_ACC:
		desc = "UFP Audio accessory";
		break;
	case PD_CC_UFP_DEBUG_ACC:
		desc = "UFP Debug accessory";
		break;
	case PD_CC_UFP_ATTACHED:
		desc = "UFP attached";
		break;
	case PD_CC_DFP_DEBUG_ACC:
		desc = "DFP Debug accessory";
		break;
	case PD_CC_DFP_ATTACHED:
		desc = "DFP attached";
		break;
	default:
		desc = "UNKNOWN";
		break;
	}
	printf("CC State: %s\n", desc);

	if (r->dp_pin) {
		switch (r->dp_pin) {
		case MODE_DP_PIN_A:
			desc = "A";
			break;
		case MODE_DP_PIN_B:
			desc = "B";
			break;
		case MODE_DP_PIN_C:
			desc = "C";
			break;
		case MODE_DP_PIN_D:
			desc = "D";
			break;
		case MODE_DP_PIN_E:
			desc = "E";
			break;
		case MODE_DP_PIN_F:
			desc = "F";
			break;
		default:
			desc = "UNKNOWN";
			break;
		}
		printf("DP pin mode: %s\n", desc);
	}

	if (r->mux_state) {
		printf("MUX: USB=%d DP=%d POLARITY=%s HPD_IRQ=%d HPD_LVL=%d\n"
		       "     SAFE=%d TBT=%d USB4=%d\n",
		       !!(r->mux_state & USB_PD_MUX_USB_ENABLED),
		       !!(r->mux_state & USB_PD_MUX_DP_ENABLED),
			(r->mux_state & USB_PD_MUX_POLARITY_INVERTED) ?
						"INVERTED" : "NORMAL",
		       !!(r->mux_state & USB_PD_MUX_HPD_IRQ),
		       !!(r->mux_state & USB_PD_MUX_HPD_LVL),
		       !!(r->mux_state & USB_PD_MUX_SAFE_MODE),
		       !!(r->mux_state & USB_PD_MUX_TBT_COMPAT_ENABLED),
		       !!(r->mux_state & USB_PD_MUX_USB4_ENABLED));
	}

	printf("Port events: 0x%08x\n", r->events);

	if (r->sop_revision)
		printf("SOP  PD Rev: %d.%d\n",
		       PD_STATUS_REV_GET_MAJOR(r->sop_revision),
		       PD_STATUS_REV_GET_MINOR(r->sop_revision));

	if (r->sop_prime_revision)
		printf("SOP' PD Rev: %d.%d\n",
		       PD_STATUS_REV_GET_MAJOR(r->sop_prime_revision),
		       PD_STATUS_REV_GET_MINOR(r->sop_prime_revision));

	for (i = 0; i < r->source_cap_count; i++) {
		/*
		 * Bits 31:30 always indicate the type of PDO
		 *
		 * Table 6-7 PD Rev 3.0 Ver 2.0
		 */
		uint32_t pdo = r->source_cap_pdos[i];
		int pdo_type = pdo & PDO_TYPE_MASK;

		if (i == 0)
			printf("Source Capabilities:\n");

		if (pdo_type == PDO_TYPE_FIXED) {
			print_pdo_fixed(pdo);
			printf("\n");
		} else if (pdo_type == PDO_TYPE_BATTERY) {
			print_pdo_battery(pdo);
		} else if (pdo_type == PDO_TYPE_VARIABLE) {
			print_pdo_variable(pdo);
		} else {
			print_pdo_augmented(pdo);
		}
	}

	for (i = 0; i < r->sink_cap_count; i++) {
		/*
		 * Bits 31:30 always indicate the type of PDO
		 *
		 * Table 6-7 PD Rev 3.0 Ver 2.0
		 */
		uint32_t pdo = r->sink_cap_pdos[i];
		int pdo_type = pdo & PDO_TYPE_MASK;

		if (i == 0)
			printf("Sink Capabilities:\n");

		if (pdo_type == PDO_TYPE_FIXED) {
			print_pdo_fixed(pdo);
			/* Note: FRS bits are reserved in PD 2.0 spec */
			printf("%s\n", pdo & PDO_FIXED_FRS_CURR_MASK ?
			       "FRS" : "");
		} else if (pdo_type == PDO_TYPE_BATTERY) {
			print_pdo_battery(pdo);
		} else if (pdo_type == PDO_TYPE_VARIABLE) {
			print_pdo_variable(pdo);
		} else {
			print_pdo_augmented(pdo);
		}
	}

	return 0;
}

int cmd_reboot_ap_on_g3(int argc, char *argv[])
{
	struct ec_params_reboot_ap_on_g3_v1 p;
	int rv;
	char *e;
	int cmdver;

	if (argc < 2) {
		p.reboot_ap_at_g3_delay = 0;
	} else {
		p.reboot_ap_at_g3_delay = strtol(argv[1], &e, 0);
		if (e && *e) {
			fprintf(stderr, "invalid number\n");
			return -1;
		}
	}
	if (ec_cmd_version_supported(EC_CMD_REBOOT_AP_ON_G3, 1))
		cmdver = 1;
	else
		cmdver = 0;

	rv = ec_command(EC_CMD_REBOOT_AP_ON_G3, cmdver, &p, sizeof(p), NULL, 0);
	return (rv < 0 ? rv : 0);
}


//========================= Chrome EC command end ==============================
#endif



//==============================================================================
/** @brief A handler for an `ectool` command.  */
struct command {
	/** The name of the command. */
	const char *name;

	/**
	 * The function to handle the command.
	 *
	 * @param argc The length of `argv`
	 * @param argv The arguments passed, including the command itself but
	 *             not 'ectool'.
	 * @return 0 if successful, or a negative `enum ec_status` value.
	 */
	int (*handler)(int argc, char *argv[]);
};


//==============================================================================
/* NULL-terminated list of commands */
const struct command Tool_Cmd_Array[] = {
	{"adcread", cmd_adc_read},
	//{"addentropy", cmd_add_entropy},
	{"apreset", cmd_apreset},
	{"autofanctrl", cmd_thermal_auto_fan_ctrl},
	//{"backlight", cmd_lcd_backlight},
	//{"battery", cmd_battery},
	//{"batterycutoff", cmd_battery_cut_off},
	//{"batteryparam", cmd_battery_vendor_param},
	{"boardversion", cmd_board_version},
	//{"button", cmd_button},
	//{"cbi", cmd_cbi},
	//{"chargecurrentlimit", cmd_charge_current_limit},
	//{"chargecontrol", cmd_charge_control},
	//{"chargeoverride", cmd_charge_port_override},
	//{"chargestate", cmd_charge_state},
	{"chipinfo", cmd_chipinfo},
	//{"cmdversions", cmd_cmdversions},
	//{"console", cmd_console},
	//{"cec", cmd_cec},
	//{"echash", cmd_ec_hash},
	//{"eventclear", cmd_host_event_clear},
	//{"eventclearb", cmd_host_event_clear_b},
	//{"eventget", cmd_host_event_get_raw},
	//{"eventgetb", cmd_host_event_get_b},
	//{"eventgetscimask", cmd_host_event_get_sci_mask},
	//{"eventgetsmimask", cmd_host_event_get_smi_mask},
	//{"eventgetwakemask", cmd_host_event_get_wake_mask},
	//{"eventsetscimask", cmd_host_event_set_sci_mask},
	//{"eventsetsmimask", cmd_host_event_set_smi_mask},
	//{"eventsetwakemask", cmd_host_event_set_wake_mask},
    {"ecupdate", cmd_flash_ec},
    {"ecbackup", cmd_backup_ec},
    
	//{"extpwrlimit", cmd_ext_power_limit},
	{"fanduty", cmd_fanduty},
	//{"flasherase", cmd_flash_erase},
	//{"flasheraseasync", cmd_flash_erase},
	//{"flashprotect", cmd_flash_protect},
	{"flashread", cmd_flash_read},
	//{"flashwrite", cmd_flash_write},
	{"flashinfo", cmd_flash_info},
	//{"flashspiinfo", cmd_flash_spi_info},
	//{"flashpd", cmd_flash_pd},
	//{"forcelidopen", cmd_force_lid_open},
	//{"fpcontext", cmd_fp_context},
	//{"fpencstatus", cmd_fp_enc_status},
	//{"fpframe", cmd_fp_frame},
	//{"fpinfo", cmd_fp_info},
	//{"fpmode", cmd_fp_mode},
	//{"fpseed", cmd_fp_seed},
	//{"fpstats", cmd_fp_stats},
	//{"fptemplate", cmd_fp_template},
	{"gpioget", cmd_gpio_get},
	{"gpioset", cmd_gpio_set},
	//{"hangdetect", cmd_hang_detect},
	{"hello", cmd_hello},
	//{"hibdelay", cmd_hibdelay},
	//{"hostevent", cmd_hostevent},
	//{"hostsleepstate", cmd_hostsleepstate},
	//{"locatechip", cmd_locate_chip},
	//{"i2cprotect", cmd_i2c_protect},
	//{"i2cread", cmd_i2c_read},
	//{"i2cwrite", cmd_i2c_write},
	//{"i2cxfer", cmd_i2c_xfer},
	//{"infopddev", cmd_pd_device_info},
	//{"inventory", cmd_inventory},
	{"led", cmd_led},
	{"loginfo", cmd_log_info},
	{"logread", cmd_read_8k_log},
	{"loganalyse", cmd_analysis_log},
	{"mfgdataread", cmd_mfg_data_read},
	{"mfgdatawrite", cmd_mfg_data_write},
	//{"lightbar", cmd_lightbar},
	//{"kbfactorytest", cmd_keyboard_factory_test},
	//{"kbid", cmd_kbid},
	//{"kbinfo", cmd_kbinfo},
	//{"kbpress", cmd_kbpress},
	//{"keyconfig", cmd_keyconfig},
	//{"keyscan", cmd_keyscan},
	//{"mkbpget", cmd_mkbp_get},
	//{"mkbpwakemask", cmd_mkbp_wake_mask},
	//{"motionsense", cmd_motionsense},
	//{"nextevent", cmd_next_event},
	//{"panicinfo", cmd_panic_info},
	//{"pause_in_s5", cmd_s5},
	//{"pdgetmode", cmd_pd_get_amode},
	//{"pdsetmode", cmd_pd_set_amode},
	//{"port80read", cmd_port80_read},
	//{"pdlog", cmd_pd_log},
	//{"pdcontrol", cmd_pd_control},
	//{"pdchipinfo", cmd_pd_chip_info},
	//{"pdwritelog", cmd_pd_write_log},
	{"pwrbtnstart", cmd_pwrbtn_test_start},
	{"pwrbtnend", cmd_pwrbtn_test_end},
	{"powerinfo", cmd_power_info},
	//{"protoinfo", cmd_proto_info},
	//{"pse", cmd_pse},
	//{"pstoreinfo", cmd_pstore_info},
	//{"pstoreread", cmd_pstore_read},
	//{"pstorewrite", cmd_pstore_write},
	{"pwmgetfanrpm", cmd_pwm_get_fan_rpm},
	//{"pwmgetkblight", cmd_pwm_get_keyboard_backlight},
	{"pwmgetnumfans", cmd_pwm_get_num_fans},
	//{"pwmgetduty", cmd_pwm_get_duty},
	{"pwmsetfanrpm", cmd_pwm_set_fan_rpm},
	//{"pwmsetkblight", cmd_pwm_set_keyboard_backlight},
	//{"pwmsetduty", cmd_pwm_set_duty},
	//{"rand", cmd_rand},
	//{"readtest", cmd_read_test},

	{"rebootec", cmd_reboot_ec},
	//{"rollbackinfo", cmd_rollback_info},
	{"rtcget", cmd_rtc_get},
	{"rtcgetalarm", cmd_rtc_get_alarm},
	{"rtcset", cmd_rtc_set},
	{"rtcsetalarm", cmd_rtc_set_alarm},
	//{"rwhashpd", cmd_rw_hash_pd},
	//{"rwsig", cmd_rwsig},
	//{"rwsigaction", cmd_rwsig_action_legacy},
	//{"rwsigstatus", cmd_rwsig_status},
	//{"sertest", cmd_serial_test},
	//{"smartdischarge", cmd_smart_discharge},
	//{"stress", cmd_stress_test},
	{"sysinfo", cmd_sysinfo},
	//{"port80flood", cmd_port_80_flood},
	{"switches", cmd_switches},
	{"temps", cmd_temperature},
	{"tempsinfo", cmd_temp_sensor_info},
	//{"test", cmd_test},
	{"thermalget", cmd_thermal_get_threshold},
	//{"thermalset", cmd_thermal_set_threshold},
	//{"tpselftest", cmd_tp_self_test},
	//{"tpframeget", cmd_tp_frame_get},
	//{"tmp006cal", cmd_tmp006cal},
	//{"tmp006raw", cmd_tmp006raw},
	//{"typeccontrol", cmd_typec_control},
	//{"typecdiscovery", cmd_typec_discovery},
	{"typecstatus", cmd_typec_status},
	//{"uptimeinfo", cmd_uptimeinfo},
	//{"usbchargemode", cmd_usb_charge_set_mode},
	//{"usbmux", cmd_usb_mux},
	//{"usbpd", cmd_usb_pd},
	//{"usbpdmuxinfo", cmd_usb_pd_mux_info},
	//{"usbpdpower", cmd_usb_pd_power},
	{"version", cmd_version},
	{"writelog", cmd_write_log},
	//{"waitevent", cmd_wait_event},
	//{"wireless", cmd_wireless},
	{"reboot_ap_on_g3", cmd_reboot_ap_on_g3},
	{"Last_Cmd", void_function}
};

const char help_str[] =
	"Commands:\n"
	"  adcread <channel>\n"
	"      Read an ADC channel.\n"
	"  apreset\n"
	"      Issue AP reset\n"
	"  autofanctrl <on>\n"
	"      Turn on automatic fan speed control.\n"
//	"  battery\n"
//	"      Prints battery info\n"
//	"  batterycutoff [at-shutdown]\n"
//	"      Cut off battery output power\n"
	"  boardversion\n"
	"      Prints the board version\n"
	"  chipinfo\n"
	"      Prints chip info\n"
    "  ecupdate\n"
    "      ecupdate ec.bin\n"
	"  ecbackup\n"
	"      ecbackup backup.bin\n"
	"  fanduty <percent>\n"
	"      Forces the fan PWM to a constant duty cycle\n"
	"  flashread <offset> <size> <outfile>\n"
	"      Reads from EC flash to a file\n"
	"  flashinfo\n"
	"      Prints information on the EC flash\n"
	"  gpioget <GPIO name>\n"
	"      Get the value of GPIO signal\n"
	"  gpioset <GPIO name>\n"
	"      Set the value of GPIO signal\n"
	"  hello\n"
	"      Checks for basic communication with EC\n"
	"  led <name> <query | auto | off | <color> | <color>=<value>...>\n"
	"      Set the color of an LED or query brightness range\n"
	"  loginfo\n"
	"  	   read shutdown case and wakeup case from eflash"
	"  logread\n"
    "      read flash log\n"
    "  loganalyse\n"
    "      analyse flash log shutdown/wakeup \n"
    "  mfgdataread\n"
    "      read mfg data \n"
    "  mfgdatawrite\n"
    "      write mfg data \n"
	"  pwrbtnstart\n"
	"      detect power button rising and falling count start"
	"  pwrbtnend\n"
	"      detect power button rising and falling count end"
	"  powerinfo\n"
	"      Prints power-related information\n"
	"  pwmgetfanrpm [<index> | all]\n"
	"      Prints current fan RPM\n"
	"  pwmgetnumfans\n"
	"      Prints the number of fans present\n"
	"  pwmsetfanrpm <targetrpm>\n"
	"      Set target fan RPM\n"
    
	"  rebootec <RO|RW|cold|cold-ap-off|hibernate|hibernate-clear-ap-off|disable-jump>"
			" [at-shutdown|switch-slot]\n"
	"      Reboot EC to RO or RW\n"
	"  rtcget\n"
	"      Print real-time clock\n"
	"  rtcgetalarm\n"
	"      Print # of seconds before real-time clock alarm goes off.\n"
	"  rtcset <time>\n"
	"      Set real-time clock\n"
	"  rtcsetalarm <sec>\n"
	"      Set real-time clock alarm to go off in <sec> seconds\n"
	"  sysinfo [flags|reset_flags|firmware_copy]\n"
	"      Display system info.\n"
	"  switches\n"
	"      Prints current EC switch positions\n"
	"  temps <sensorid>\n"
	"      Print temperature.\n"
	"  tempsinfo <sensorid>\n"
	"      Print temperature sensor info.\n"
	"  thermalget <platform-specific args>\n"
	"      Get the threshold temperature values from the thermal engine.\n"
	"  typecstatus <port>\n"
	"      Get status information for port\n"
	"  version\n"
	"      Prints EC version\n"
	"  writelog\n"
	"  	   write log to eflash\n"
	"  reboot_ap_on_g3\n"
	"      Requests that the EC will automatically reboot the AP the next time\n"
	"      we enter the G3 power state.\n"
	"";


void help(void)
{
    printf("==============================================================\n");
    printf("=                Chrome EC tool Version : %s               =\n",TOOLS_VER);
    printf("=       (C)Copyright %s Telecom Technology Co.,Ltd      =\n",Vendor);
    printf("=                     All Rights Reserved.                   =\n");
    printf("=                                        --%s       =\n",__DATE__);
    printf("=                                                            =\n");
    printf("=                   Modified by Morgen                       =\n");
    printf("==============================================================\n");
    printf("%s", help_str);
}


int main(int argc, char *argv[])
{
    bool IOInitOK;
    uint32_t i;
    uint8_t  EC_Tool_Cmd=0;
    char **ToolArgv;
    char ToolArgc;

    #if 0
    printf("cmd argc=%d\n", argc);

    for(i=0; i<argc; i++)
    {
        printf("cmd argv[%d] = %s\n", i, argv[i]);
    }
    #endif
    
    if(1 == argc)
    {
        goto ArgcError;
    }

    // clrscr(); clear screen. only in the TC (conio.h)
    // other C is  system("cls")  (stdlib.h);
    //system("cls");

    // Step-1 Looking for command
    EC_Tool_Cmd = 0;
    i=0;
    while(1)
    {
        if(!strcmp(Tool_Cmd_Array[i].name, argv[1]))
        {
            EC_Tool_Cmd = i;
            ToolArgv = &argv[1]; // Offset to the first parameter of the cmd
            ToolArgc = argc -1;  // Adjust the number of parameters
            break;
        }
        
        if(!strcmp(Tool_Cmd_Array[i].name, "Last_Cmd"))
        {
            printf("This command is not found\n\n\n");
            goto ArgcError;
        }
        i++;
    }

    // Step-2 Init winio for IO access
    IOInitOK = InitializeWinIo();
    if(IOInitOK)
    {
        //printf("WinIo OK.\n");
    }
    else
    {
        printf("Error during initialization of WinIo.\n");
        goto IOError;
    }

    // Step-3 Init lpc
    if(0 != comm_init_lpc())
    {
        goto CmdError;
    }

    // Step-4 Handle commands
    if(0!=Tool_Cmd_Array[EC_Tool_Cmd].handler(ToolArgc, ToolArgv))
    {
        goto CmdError;
    }

	goto end;

ArgcError:
    help();
    ShutdownWinIo();
    return 1;
    
IOError:
    ShutdownWinIo();
    return 1;
    
CmdError:
    ShutdownWinIo();
    return 1;
    
end:
ShutdownWinIo();
    return 0;

}
