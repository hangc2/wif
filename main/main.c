//*****************************************************************************
// main.c
//
// Reference code to demonstrate getting the current time using an NTP server.
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup tftp
//! @{
//
//****************************************************************************

//Standard includes
#include <stdlib.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"

// Standard includes
#include "simplelink.h"

// OS includes
#include "osi.h"

//Common includes

#include "network_if.h"
#include "uart_if.h"
#include "udma_if.h"
#include "common.h"

//App includes
#include "pinmux.h"
#include "datatypes.h"
#include "tftp.h"


//*****************************************************************************
//                      GLOBAL VARIABLES for VECTOR TABLE
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
extern void (* const g_pfnVectors[])(void);

//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
#define APP_NAME		"TFTP Client"
#define TFTP_IP			0xC0A8000F	/* This is the host IP */
#define FILE_SIZE_MAX	(20*1024)	/* Max File Size set to 20KB */
#define SSID 			"Orange Monster"
#define SSID_KEY		"9874123!"
#define TFTP_READ		1		/* Set '1' to read else '0' */
#define TFTP_WRITE		1		/* Set '1' to write else '0' */

#define SLEEP_TIME              80000000
#define OSI_STACK_SIZE          2048


//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************

static void
DisplayBanner(char * AppName)
{
    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t	  CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs) || defined(gcc)
    IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
#if 0
bool get_wifi_info(char* ssid, SlSecParams_t &secPrams)
{
  bool ret_val = TRUE;
  char* ssid = NULL;
  char* security_type=NULL;
  do
  {
      ret_val = FALSE;

      UART_PRINT("\n\r\n\rPlease enter the AP(open) SSID name # ");

      //
      // Get the AP name to connect over the UART
      //
      lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
      if(lRetVal > 0)
      {
          // remove start/end spaces if any
          lRetVal = TrimSpace(acCmdStore);

          //
          // Parse the AP name
          //
          strncpy(ssid, acCmdStore, lRetVal);
          if(ssid != NULL)
          {
              ret_val = TRUE;
              ssid[lRetVal] = '\0';
          }
      }
  }while(ret_val == FALSE);

  do
  {
      ret_val = FALSE;

      UART_PRINT("\n\r\n\rPlease enter the AP serurity type 0=OPEN | 2=WPA2|WEP # ");

      //
      // Get the AP name to connect over the UART
      //
      lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
      if(lRetVal > 0)
      {
          // remove start/end spaces if any
          lRetVal = TrimSpace(acCmdStore);

          //
          // Parse the AP name
          //
          strncpy(security_type, acCmdStore, lRetVal);
          if(pcSsid != NULL)
          {
              if(pcSsid == '')
              ret_val = TRUE;
              pcSsid[lRetVal] = '\0';
          }
      }
  }while(ret_val == FALSE);

  return ret_val;
}
#endif

static void TFTPTask(void *pvParameters)
{
    SlSecParams_t secParams;
    unsigned char *pucFileBuffer = NULL;  // Data read or to be written
    unsigned long uiFileSize;

    char *FileRead = "readFromServer.txt";  // File to be read using TFTP. Change string to filename
    char *FileWrite = "writeToServer.txt"; // File to be written using TFTP. Change string to filename.

    long pFileHandle;			// Pointer to file handle
    SlFsFileInfo_t pFsFileInfo;
    long lRetVal = -1;
    unsigned short uiTftpErrCode;


//get wifi information
  //  get_wifi_info(ap_ssid, secPrams);

    // Configuring security parameters for the AP
    secParams.Key = SSID_KEY;
    secParams.KeyLen = 30;
    secParams.Type = SL_SEC_TYPE_WPA_WPA2;

    lRetVal = Network_IF_InitDriver(ROLE_STA);



    // Connecting to WLAN AP - Set with static parameters defined at the top
    // After this call we will be connected and have IP address
    lRetVal = Network_IF_ConnectAP(SSID,secParams);

    UART_PRINT("Connecting to TFTP server %d.%d.%d.%d\n\r",\
                  SL_IPV4_BYTE(TFTP_IP, 3),SL_IPV4_BYTE(TFTP_IP, 2),
                  SL_IPV4_BYTE(TFTP_IP, 1),SL_IPV4_BYTE(TFTP_IP, 0));
    if(TFTP_READ)
    {
        uiFileSize = FILE_SIZE_MAX;

        pucFileBuffer = malloc(uiFileSize);
        if(NULL == pucFileBuffer)
        {
            UART_PRINT("Can't Allocate Resources\r\n");
            LOOP_FOREVER();
        }

        memset(pucFileBuffer,'\0',uiFileSize);

        lRetVal = sl_TftpRecv(TFTP_IP, FileRead, (char *)pucFileBuffer,\
                                &uiFileSize, &uiTftpErrCode );
        UART_PRINT("file read is %s\n", pucFileBuffer);

        if(lRetVal < 0)
        {
            free(pucFileBuffer);
            ERR_PRINT(lRetVal);
            LOOP_FOREVER();
        }

        lRetVal = sl_FsGetInfo((unsigned char *)FileRead, NULL, &pFsFileInfo);

        if(lRetVal < 0 )
            lRetVal = sl_FsOpen((unsigned char *)FileRead,\
                    FS_MODE_OPEN_CREATE(FILE_SIZE_MAX,_FS_FILE_OPEN_FLAG_COMMIT|\
                      _FS_FILE_PUBLIC_WRITE), NULL,&pFileHandle);
        else
            lRetVal = sl_FsOpen((unsigned char *)FileRead,FS_MODE_OPEN_WRITE, \
                                NULL,&pFileHandle);

        if(lRetVal < 0)
        {
            free(pucFileBuffer);
            ERR_PRINT(lRetVal);
            LOOP_FOREVER();
        }

        lRetVal = sl_FsWrite(pFileHandle,0, pucFileBuffer, uiFileSize);

        if(lRetVal < 0)
        {
            free(pucFileBuffer);
            lRetVal = sl_FsClose(pFileHandle,0,0,0);
            ERR_PRINT(lRetVal);
            LOOP_FOREVER();
        }

        UART_PRINT("TFTP Read Successful \r\n");

        lRetVal = sl_FsClose(pFileHandle,0,0,0);

    }

    if(TFTP_WRITE)
    {
    	/* open same file which has been written with the server's file content */
        lRetVal = sl_FsOpen((unsigned char *)FileRead,FS_MODE_OPEN_READ, \
                                NULL,&pFileHandle);
        if(lRetVal < 0)
        {
            free(pucFileBuffer);
            ERR_PRINT(lRetVal);
            LOOP_FOREVER();
        }

        lRetVal = sl_FsGetInfo((unsigned char *)FileRead, NULL, &pFsFileInfo);
        if(lRetVal < 0)
        {
            lRetVal = sl_FsClose(pFileHandle,0,0,0);
            free(pucFileBuffer);
            ERR_PRINT(lRetVal);
            LOOP_FOREVER();
        }

        uiFileSize = (&pFsFileInfo)->FileLen;

        lRetVal = sl_FsRead(pFileHandle, 0,  pucFileBuffer, uiFileSize);
        if(lRetVal < 0)
        {
            lRetVal = sl_FsClose(pFileHandle,0,0,0);
            free(pucFileBuffer);
            ERR_PRINT(lRetVal);
            LOOP_FOREVER();
        }

        lRetVal = sl_FsClose(pFileHandle,0,0,0);
        /* write to server with different file name */
        lRetVal = sl_TftpSend(TFTP_IP, FileWrite,(char *) pucFileBuffer,\
                            &uiFileSize, &uiTftpErrCode  );
        if(lRetVal < 0)
        {
            free(pucFileBuffer);
            ERR_PRINT(lRetVal);
            LOOP_FOREVER();
        }

        UART_PRINT("TFTP Write Successful \r\n");
    }

    LOOP_FOREVER();
}

enum cmd_str{
	TFTP,
	CAMERA,
	INVALID_CMD
};

#define CMD_LEN 10

/*  trasform str to int  */

uint8 str_to_int(char* str)
{
	//  0xFF is not valid number
	uint8 num = 0xFF;
	if(str==NULL)
	{
		return num;
	}
	while(*str!='\n')
	{

	}
}

static void cmd_dispatch(void *pvParameters)
{
	cmd_ptr cmd = INVALID_CMD;
	uint8 rcv_cmd = 0;
	char input_str[CMD_LEN];
	int8 ret_val = 0;
	do{
		//  get cmd from prompt
        do
        {
        	rcv_cmd = 0;

            UART_PRINT("\n\r\n\rPlease enter CMD # ");

            //
            // Get the command over the UART
            //
            ret_val = GetCmd(input_str, sizeof(input_str));
            if(ret_val > 0)
            {
                // remove start/end spaces if any
                ret_val = TrimSpace(input_str);

                //
                // Parse the cmd
                //
                strncpy(pcSsid, acCmdStore, lRetVal);
                if(input_str[1] != NULL)
                {
                    ucRecvdAPDetails = 1;
                    pcSsid[lRetVal] = '\0';

                }
            }
        }while(rcv_cmd == 0);

		UART_PRINT("CMD Dispatch#\n");

	}while(1);
}

//****************************************************************************
//							MAIN FUNCTION
//****************************************************************************
void main()
{
    long lRetVal = -1;
    //
    // Board Initialization
    //
    BoardInit();

    //
    // Enable and configure DMA
    //
    UDMAInit();
    //
    // Pinmux for UART
    //
    PinMuxConfig();
    //
    // Configuring UART
    //
    InitTerm();
    //
    // Display Application Banner
    //
    DisplayBanner(APP_NAME);
    //
    // Start the SimpleLink Host
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    //
    // Start the Receiving file
    //

    lRetVal = osi_TaskCreate(TFTPTask,
                    (const signed char *)"TFTP",
                    OSI_STACK_SIZE,
                    NULL,
                    1,
                    NULL );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the task scheduler
    //
    osi_start();

}