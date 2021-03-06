/*******************************************************************************
** File: tim_app.c
**
** Purpose:
**   This file contains the source code for the TIM App.
**
*******************************************************************************/
/*
**   Include Files:
*/

#include "tim_app.h"
#include "tim_app_perfids.h"
#include "tim_app_msgids.h"
#include "tim_app_msg.h"
#include "tim_app_events.h"
#include "tim_app_version.h"
#include "tim_serial_out.h"

/*
** global data
*/

tim_hk_tlm_t       TIM_HkTelemetryPkt;
CFE_SB_PipeId_t    TIM_CommandPipe;
CFE_SB_MsgPtr_t    TIMMsgPtr;


/*
** Serial Status Data
*/
serial_out_t TIM_SerialUSB;
int serial_busy;
int serial_last_sent;

#define TIM_TEMP_MAX_COUNT             5
int temp_wait_count;
int timer_event_last;

#define TIM_IMAGE_MAX_COUNT             3
int image_wait_count;
int image_event_last;


/*
** globals for poweroff timer
*/ 
uint32 poweroff_timer_id;
uint32 poweroff_timer_clock_accuracy = 100000; /* 100,000 microseconds resolution -- should use very little clock time with low resolution */


/*
** global data for child task (cameraman) use
*/
//uint32             	TIM_ChildTaskID;
//uint32			    Cameraman_QueueID;

static CFE_EVS_BinFilter_t  TIM_EventFilters[] =
       {  /* Event ID    mask */
          {TIM_STARTUP_INF_EID,       0x0000},
          {TIM_COMMAND_ERR_EID,       0x0000},
          {TIM_COMMANDNOP_INF_EID,    0x0000},
          {TIM_COMMANDRST_INF_EID,    0x0000},
       };

/*
** Prototype Section
*/


/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* TIM_AppMain() -- Application entry point and main process loop             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void TIM_AppMain( void )
{
    int32  status;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(TIM_APP_PERF_ID);

    TIM_AppInit();

    /*
    ** TIM Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(TIM_APP_PERF_ID);

        /* Pend on receipt of command packet -- timeout set to 500 millisecs */
        status = CFE_SB_RcvMsg(&TIMMsgPtr, TIM_CommandPipe, 500);
        
        CFE_ES_PerfLogEntry(TIM_APP_PERF_ID);

        TIM_UpdateTimerEvent();

        if (status == CFE_SUCCESS)
        {
            TIM_ProcessCommandPacket();
        }
    }

    CFE_ES_ExitApp(RunStatus);

} /* End of TIM_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* TIM_AppInit() --  initialization                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void TIM_AppInit(void)
{
    /*
    ** Disable debugging prints
    */
    //OS_printf_disable();

	/*
	** Register the app with Executive services
	*/
	CFE_ES_RegisterApp() ;

	/*
	** Register the events
	*/
	CFE_EVS_Register(TIM_EventFilters,
			sizeof(TIM_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
			CFE_EVS_BINARY_FILTER);

	/*
	** Create the Software Bus command pipe and subscribe to housekeeping
	**  messages
	*/
	CFE_SB_CreatePipe(&TIM_CommandPipe, TIM_PIPE_DEPTH,"TIM_CMD_PIPE");
	CFE_SB_Subscribe(TIM_APP_CMD_MID, TIM_CommandPipe);
	CFE_SB_Subscribe(TIM_APP_SEND_HK_MID, TIM_CommandPipe);

	TIM_ResetCounters();

	serial_busy = 0;
    serial_last_sent = 0;
    
    temp_wait_count = 0;
    timer_event_last = 0;
    
    image_wait_count = 0;
    image_event_last = 0;

    if(serial_out_init(&TIM_SerialUSB, (char *) SERIAL_OUT_PORT) < 0)
    {
        OS_printf("TIM: error, serial out init failed.\n");
    }

    //serial_out_init(&TIM_SerialUSB, SERIAL_OUT_PORT);
    
    TIM_Parallel_Init();

    /*
    ** Initialize Timer for poweroff -- attaches to timer_callback_poweroff_system()
    */
    //OS_printf("TIM_APP: Attempting to initialize reboot timer.\n");
    int32 timer_ret = OS_TimerCreate( (uint32 *)&poweroff_timer_id, (const char *)"poweroff_timer", (uint32 *)&poweroff_timer_clock_accuracy, (OS_TimerCallback_t) &timer_callback_poweroff_system );
    if(timer_ret != OS_SUCCESS)
    {
        OS_printf("OS_TimerCreate() failed to attach timer to timer_callback_poweroff_system() function! returned: %d\n", timer_ret);
    }
    uint32 poweroff_interval = 10000000 ;   // 10 seconds in microseconds -- function should never be called twice, but this is a failsafe of sorts.
    OS_printf("TIM_APP: Attempting to set reboot timer to %d microseconds.\n", POWEROFF_TIME);
    timer_ret = OS_TimerSet( (uint32)poweroff_timer_id, (uint32) POWEROFF_TIME, (uint32)poweroff_interval );
    if(timer_ret != OS_SUCCESS)
    {
        OS_printf("OS_TimerSet() failed to set start time for timer_callback_poweroff_system() function! returned: %d\n", timer_ret);
    }
    //TIM_ChildInit();

	CFE_SB_InitMsg(&TIM_HkTelemetryPkt,
			TIM_APP_HK_TLM_MID,
			TIM_APP_HK_TLM_LNGTH, TRUE);

	CFE_EVS_SendEvent (TIM_STARTUP_INF_EID, CFE_EVS_INFORMATION,
			"TIM App Initialized. Version %d.%d.%d.%d",
			TIM_APP_MAJOR_VERSION,
			TIM_APP_MINOR_VERSION, 
			TIM_APP_REVISION, 
			TIM_APP_MISSION_REV);

} /* End of TIM_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  TIM_ProcessCommandPacket                                         */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the TIM     */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void TIM_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(TIMMsgPtr);

    switch (MsgId)
    {
        case TIM_APP_CMD_MID:
            TIM_ProcessGroundCommand();
            break;

        case TIM_APP_SEND_HK_MID:
            TIM_ReportHousekeeping();
            break;

        default:
            TIM_HkTelemetryPkt.tim_command_error_count++;
            CFE_EVS_SendEvent(TIM_COMMAND_ERR_EID,CFE_EVS_ERROR,
			"TIM: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End TIM_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* TIM_ProcessGroundCommand() -- TIM ground commands                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void TIM_ProcessGroundCommand(void)
{
	uint16 CommandCode;

	CommandCode = CFE_SB_GetCmdCode(TIMMsgPtr);

	/* Process "known" TIM app ground commands */
	switch (CommandCode)
	{
	case TIM_APP_NOOP_CC:
		TIM_HkTelemetryPkt.tim_command_count++;
		CFE_EVS_SendEvent(TIM_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
			"TIM: NOOP command");
		break;

	case TIM_APP_RESET_COUNTERS_CC:
		TIM_ResetCounters();
		break;
	
/********************************************************************************************
***********************************************************************************************
*********************************************************************************************/
	case TIM_APP_SEND_IMAGE0_CC:
            /*
            ** Recieved Send Image File Command from Camera 0
            */
            if(serial_last_sent == TIM_APP_SEND_TEMPS_CC)
            {
                if (serial_busy == 0)
                {
                    serial_busy = 1;
                    serial_last_sent = TIM_APP_SEND_IMAGE0_CC;
                    TIM_SendImageFile();
                    serial_busy = 0;
                    
                    /*
                    ** Reset Wait Count when we know its been sent
                    */
                    image_wait_count = 0;

                    TIM_HkTelemetryPkt.tim_command_count++;
                    TIM_HkTelemetryPkt.tim_command_image_count++;
                    
                    CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                        "TIM: Send Camera 0 File Command Received and Sent");
                }
                else
                {
                    /*
                    ** It was time to send a picture but serial busy
                    */
                    CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                        "TIM: Send Camera 0 File Command Received but Serial Busy");
                }
            }
            else
            {
                /*
                ** Increment Image Delay/Wait Count
                ** 
                ** TIM_IMAGE_MAX_COUNT  =   3
                */
                image_wait_count++;
                
                if(image_wait_count >= TIM_IMAGE_MAX_COUNT)
                {
                    /*
                    ** if max wait has been reached, try to send an image
                    */
                    if (serial_busy == 0)
                    {
                        serial_busy = 1;
                        serial_last_sent = TIM_APP_SEND_IMAGE0_CC;
                        TIM_SendImageFile();
                        serial_busy = 0;
                        
                        CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                            "TIM: Send Camera 0 File Command Received and Sent");
                            
                        TIM_HkTelemetryPkt.tim_command_count++;
                        TIM_HkTelemetryPkt.tim_command_image_count++;
                        
                        /*
                        ** Reset Wait Count when we know its been sent
                        */
                        image_wait_count = 0;
                        
                    }
                    else
                    {
                        CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                            "TIM: Send Camera 0 File Command Received but Serial Busy, WC = [%d]", image_wait_count);
                    }
                
                }
                else
                {
                     CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                                    "TIM: Send Camera 0 File Yielding, WC = [%d]", image_wait_count);
                }
            }
		    break;
/********************************************************************************************
***********************************************************************************************
*********************************************************************************************/
	case TIM_APP_SEND_IMAGE1_CC:
            /*
            ** Recieved Send Image File Command from Camera 1
            */
            if(serial_last_sent == TIM_APP_SEND_TEMPS_CC )
            {
                /*
                ** The last file sent was a temp file, 
                ** so send a picture if serial can send
                */
                if (serial_busy == 0)
                {
                    serial_busy = 1;
                    serial_last_sent = TIM_APP_SEND_IMAGE1_CC;
                    TIM_SendImageFile();
                    serial_busy = 0;
                    
                    CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                        "TIM: Send Camera 1 File Command Received");
                        
                    TIM_HkTelemetryPkt.tim_command_count++;
                    TIM_HkTelemetryPkt.tim_command_image_count++;
                    
                    /*
                    ** Reset Wait Count when we know its been sent
                    */
                    image_wait_count = 0;
                    
                }
                else
                {
                    CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                        "TIM: Send Camera 1 File Command Received but Serial Busy");
                }
                
            }
            else
            {
                /*
                ** Increment Image Delay/Wait Count
                ** 
                ** TIM_IMAGE_MAX_COUNT  =   3
                */
                image_wait_count++;
                
                if(image_wait_count >= TIM_IMAGE_MAX_COUNT)
                {
                    /*
                    ** if max wait has been reached, try to send an image
                    */
                    if (serial_busy == 0)
                    {
                        serial_busy = 1;
                        serial_last_sent = TIM_APP_SEND_IMAGE1_CC;
                        TIM_SendImageFile();
                        serial_busy = 0;
                        
                        CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                            "TIM: Send Camera 1 File Command Received");
                            
                        TIM_HkTelemetryPkt.tim_command_count++;
                        TIM_HkTelemetryPkt.tim_command_image_count++;
                        
                        /*
                        ** Reset Wait Count when we know its been sent
                        */
                        image_wait_count = 0;
                        
                    }
                    else
                    {
                        CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                            "TIM: Send Camera 1 File Command Received but Serial Busy, WC = [%d]", image_wait_count);
                    }
                
                }
                else
                {
                     CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, 
                            "TIM: Send Camera 1 File yeilding, Wait Count = [%d]", image_wait_count);
                } 
                
                
            }        
		    break;
/********************************************************************************************
***********************************************************************************************
*********************************************************************************************/
	case TIM_APP_SEND_TEMPS_CC:
            /*
            ** Send Temp File Command Received from ADS1115
            */
            if(serial_last_sent != TIM_APP_SEND_TEMPS_CC)
            {
                if (serial_busy == 0)
                {
                    serial_busy = 1;
                    serial_last_sent = TIM_APP_SEND_TEMPS_CC;
                    TIM_SendTempsFile();
                    serial_busy = 0;
                    
                    CFE_EVS_SendEvent(TIM_COMMAND_TEMPS_EID,CFE_EVS_INFORMATION,
                        "TIM: Send Temps File Command Received");
                    TIM_HkTelemetryPkt.tim_command_count++;
                    TIM_HkTelemetryPkt.tim_command_temps_count++;
                    
                    /*
                    ** Reset Wait Count when we know its been sent
                    */
                    temp_wait_count = 0;
                }
                else
                {
                    CFE_EVS_SendEvent(TIM_COMMAND_TEMPS_EID,CFE_EVS_INFORMATION,
                        "TIM: Send Temps File Command Received but Serial Busy");
                }
            }
            else
            {
                temp_wait_count++;
                /*
                ** Increment wait count to see if images are getting sent
                ** if they aren't getting sent after TIM_TEMP_MAX_COUNT (== 5?)
                */
                if(temp_wait_count >= TIM_TEMP_MAX_COUNT)
                {
                    /*
                    ** if max wait has been reached, try to send temps 
                    */
                    if(serial_busy == 0)
                    {
                        serial_busy = 1;
                        serial_last_sent = TIM_APP_SEND_TEMPS_CC;                        
                        TIM_SendTempsFile();
                        serial_busy = 0;
                        
                        CFE_EVS_SendEvent(TIM_COMMAND_TEMPS_EID,CFE_EVS_INFORMATION,
                            "TIM: Reached Temp Wait Limit. Force Sending Temps File");
                        
                        TIM_HkTelemetryPkt.tim_command_count++;
                        TIM_HkTelemetryPkt.tim_command_temps_count++;
                        
                        /*
                        ** Reset Wait Count when we know its been sent
                        */
                        temp_wait_count = 0;
                    }
                    else
                    {
                        CFE_EVS_SendEvent(TIM_COMMAND_TEMPS_EID,CFE_EVS_INFORMATION,
                            "TIM: Reached Temp Wait Limit But Serial Busy");
                    }
                }
                else
                {
                     CFE_EVS_SendEvent(TIM_COMMAND_TEMPS_EID,CFE_EVS_INFORMATION,
                        "TIM: Send Temps File Wait Count Reached [%d]", temp_wait_count);
                } 
            }
            
		    break;
/********************************************************************************************
***********************************************************************************************
*********************************************************************************************/

	/* default case already found during FC vs length test */
	default:
		CFE_EVS_SendEvent(TIM_COMMAND_ERR_EID, CFE_EVS_INFORMATION,
				"Command code not found (CC = %i)", CommandCode);
		TIM_HkTelemetryPkt.tim_command_error_count++;
		break;
    }
    return;

} /* End of TIM_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  TIM_ReportHousekeeping                                           */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void TIM_ReportHousekeeping(void)
{
    /*
	memcpy((char *) &TIM_HkTelemetryPkt.SerialQueueInfo,
		(char *) &TIM_SerialQueueInfo,
		sizeof(TIM_SerialQueue_t));
    */

	CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &TIM_HkTelemetryPkt);
	CFE_SB_SendMsg((CFE_SB_Msg_t *) &TIM_HkTelemetryPkt);
	return;

} /* End of TIM_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  TIM_ResetCounters                                                */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry. Also clears file loc data.             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void TIM_ResetCounters(void)
{
	/* Status of commands processed by the TIM App */
	TIM_HkTelemetryPkt.tim_command_count       = 0;
	TIM_HkTelemetryPkt.tim_command_error_count = 0;
	TIM_HkTelemetryPkt.tim_command_image_count = 0;
	TIM_HkTelemetryPkt.tim_command_temps_count = 0;
    memset(TIM_HkTelemetryPkt.tim_last_image_sent, '\0', sizeof(TIM_HkTelemetryPkt.tim_last_image_sent));
    memset(TIM_HkTelemetryPkt.tim_last_temps_sent, '\0', sizeof(TIM_HkTelemetryPkt.tim_last_temps_sent));

	CFE_EVS_SendEvent(TIM_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
		"TIM: RESET command");
	return;

} /* End of TIM_ResetCounters() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  TIM_SendImageFile (Generic)                                        */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function sends a picture using a JPG file, based on info      */
/*         from the global (CFE_SB_MsgPtr_t *) TIMMsgPtr                      */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void TIM_SendImageFile(void)
{   
    int os_ret_val = 0;
    //int32 os_fd = 0;
    int index = 0;
    //uint32 bytes_per_read = 1; /* const */
    //uint16 total_bytes_read = 0;
    //uint8 data_buf[2];
    //data_buf[0] = 0;
    //data_buf[1] = 0;

    //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    
    /* Creating a pointer to handle the TIMMsgPtr as TIM_IMAGE_CMD_PKT  */

    TIM_IMAGE_CMD_PKT_t *ImageCmdPtr;
    ImageCmdPtr = (TIM_IMAGE_CMD_PKT_t *) TIMMsgPtr;

    //OS_printf("Received TIM_SendImageFile() with %s\n", ImageCmdPtr->ImageName);

    char file_path[OS_MAX_PATH_LEN];

    //OS_printf("sizeof(file_path[OS_MAX_PATH_LEN]) = %d\n", sizeof(file_path));
    
    memset(file_path, '\0', sizeof(file_path));

    if((os_ret_val = snprintf(file_path, sizeof(file_path), "/ram/images/%s", ImageCmdPtr->ImageName)) < 0)
    {
        OS_printf("TIM snprintf failure: ret = %d\n", os_ret_val);
    }

    /*
    if ((os_fd = OS_open((const char * ) file_path, (int32) OS_READ_ONLY, (uint32) mode)) < OS_FS_SUCCESS)
    {
        OS_printf("TIM: OS_open Returned [%d] (expected non-negative value).\n", os_fd);
        return;
    }
    */

    /*
    ** Read 1 byte at a time
    ** Uncomment the following to o
    */
    /*
    while( OS_read((int32) os_fd, (void *) data_buf, (uint32) bytes_per_read) &&  (total_bytes_read < 20000) )
    {
        //OS_printf("From Tim Image: File '%s' Byte %.2d = %#.2X %#.2X\n", ImageCmdPtr->ImageName, total_bytes_read, data_buf[0], data_buf[1]);
        total_bytes_read++;
        //serial_write_byte(&TIM_SerialUSB, (unsigned char) data_buf[0]);
        data_buf[0] = 0;
        data_buf[1] = 0;
    }
    OS_printf("Image File Length (bytes) = [%u].\n", total_bytes_read);
    */


    //OS_printf("Image File Length (bytes) = [%u].\n", total_bytes_read);

    
    /*
    uint8 file_len[2];
    file_len[0] = *((uint8 *) &total_bytes_read);
    file_len[1] = *(((uint8 *) &total_bytes_read) + 1);
    OS_printf("Image Length MSB = [%#.2X]\n", file_len[1]);
    OS_printf("Image Length LSB = [%#.2X]\n", file_len[0]);
    //OS_printf("Image Length MSB = [%#.2X]\n", file_len[1]);
    //OS_printf("Image Length LSB = [%#.2X]\n", file_len[0]);


    OS_close(os_fd);

    /*
    ** Data prepped for serial:
    ** total_bytes_read : file size
    ** TempsCmdPtr->TempsName : file name
    ** sizeof(TempsCmdPtr->TempsName) : 22
    */

    /*
    uint8 start byte and pkt id  {0xF} {0:?, 1:image, 2:temps, 3:log, 4:unknown}
    uint16 pkt size in bytes
    uint8 filename length
    char filename[22]
    blank (0x00)
    ...
    DATA
    ...
    uint16 data_stop_flag 0xFF [filename length]
    char filename[22]
    uint8 stop byte
    */

    //OS_close(os_fd);
    
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xFF);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xF2);
    
    //serial_write_byte(&TIM_SerialUSB, (unsigned char) file_len[0]);
    //serial_write_byte(&TIM_SerialUSB, (unsigned char) file_len[1]); /* check endianess */

    for(index = 0; index < sizeof(ImageCmdPtr->ImageName); index++)
    {
        serial_write_byte(&TIM_SerialUSB, (unsigned char) (*(((char *) ImageCmdPtr->ImageName) + index)));
    }
    
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x00);

    tim_serial_write_file(&TIM_SerialUSB, (char *) file_path);

    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xFF);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xF3);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x0D);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x0A);

    CFE_EVS_SendEvent(TIM_COMMAND_IMAGE_EID,CFE_EVS_INFORMATION, "Image File \'%s\' Sent Successfully", file_path);
    
    //OS_printf("Reached end of TIM_SendImageFile().\n");

	return;
} /* End of TIM_SendImageFile() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* TIM_SendTempsFile()  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void TIM_SendTempsFile(void)
{
    int os_ret_val = 0;
    int index = 0;
    /*
    int32 os_fd = 0;
    uint32 bytes_per_read = 1;
    uint16 total_bytes_read = 0;
    uint8 data_buf[2];
    data_buf[0] = 0;
    data_buf[1] = 0;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    */

    /* Creating a pointer to handle the TIMMsgPtr as TIM_IMAGE_CMD_PKT  */
    TIM_TEMPS_CMD_PKT_t *TempsCmdPtr;
    TempsCmdPtr = (TIM_TEMPS_CMD_PKT_t *) TIMMsgPtr;

    //OS_printf("Received TIM_SendTempsFile() with %s\n", TempsCmdPtr->TempsName);

    char file_path[OS_MAX_PATH_LEN];

    //OS_printf("sizeof(file_path[OS_MAX_PATH_LEN]) = %d\n", sizeof(file_path));

    memset(file_path, '\0', sizeof(file_path));

    if((os_ret_val = snprintf(file_path, sizeof(file_path), "/ram/temps/%s", TempsCmdPtr->TempsName)) < 0)
    {
        OS_printf("TIM snprintf failure: ret = %d\n", os_ret_val);
    }
    
    /*
    if ((os_fd = OS_open((const char * ) file_path, (int32) OS_READ_ONLY, (uint32) mode)) < OS_FS_SUCCESS)
    {
        OS_printf("TIM: OS_open Returned [%d] (expected non-negative value).\n", os_fd);
        return;
    }
    */

    /*
    ** Read 1 byte at a time
    */
    /*
    
    while( OS_read((int32) os_fd, (void *) data_buf, (uint32) bytes_per_read))
    {
        //OS_printf("From Tim Temps: File '%s' Byte %.2d = %#.2X %#.2X\n", TempsCmdPtr->TempsName, total_bytes_read, data_buf[0], data_buf[1]);
        total_bytes_read++;
        //serial_write_byte(&TIM_SerialUSB, (unsigned char) data_buf[0]);
        data_buf[0] = 0;
        data_buf[1] = 0;
    }

    if(total_bytes_read != 16)
    {
        OS_printf("Temperature File shorter [%d] than expected [16].\n", total_bytes_read);
    }

    uint8 file_len[2];
    file_len[0] = *((uint8 *) &total_bytes_read);
    file_len[1] = *(((uint8 *) &total_bytes_read) + 1);
    OS_printf("Temp Length MSB = [%#.2X]\n", file_len[1]);
    OS_printf("Temp Length LSB = [%#.2X]\n", file_len[0]);
    //OS_printf("Image Length MSB = [%#.2X]\n", file_len[1]);
    //OS_printf("Image Length LSB = [%#.2X]\n", file_len[0]);

    OS_close(os_fd);
    
    */

    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xFF);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xF4);
    
    //serial_write_byte(&TIM_SerialUSB, (unsigned char) file_len[1]);
    //serial_write_byte(&TIM_SerialUSB, (unsigned char) file_len[0]); /* check endianess */

    for(index = 0; index < sizeof(TempsCmdPtr->TempsName); index++)
    {
        serial_write_byte(&TIM_SerialUSB, (unsigned char) (*(((char *) TempsCmdPtr->TempsName) + index)));
    }

    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x00);

    tim_serial_write_file(&TIM_SerialUSB, (char *) file_path);

    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xFF);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xF5);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x0D);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x0A);


    CFE_EVS_SendEvent(TIM_COMMAND_TEMPS_EID,CFE_EVS_INFORMATION, 
            "Temps File \'%s\' Sent Successfully", file_path);

	return;
} /* End of TIM_SendTempsFile() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* TIM_SendLogFile()                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void TIM_SendLogFile(void)
{
    char log_path[] = "/ram/logs/CFS_Boot.out";
    char logs_filename[] = "CFS_Boot.out";
    int index = 0;
    
    // OS_printf("TIM_APP: Sending Log file.\n");
    
    /*
    int32 os_fd = 0;
    uint32 bytes_per_read = 1; 
    uint16 total_bytes_read = 0;
    uint8 data_buf[2];
    data_buf[0] = 0;
    data_buf[1] = 0;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    if ((os_fd = OS_open((const char * ) log_path, (int32) OS_READ_ONLY, (uint32) mode)) < OS_FS_SUCCESS)
    {
        OS_printf("TIM::SendLogFile(): OS_open Returned [%d] (expected non-negative value).\n", os_fd);
        return;
    }
    */

    /*
    ** Read 1 byte at a time
    */
    /*
    while( OS_read((int32) os_fd, (void *) data_buf, (uint32) bytes_per_read))
    {
        //OS_printf("From Tim Temps: File '%s' Byte %.2d = %#.2X %#.2X\n", TempsCmdPtr->TempsName, total_bytes_read, data_buf[0], data_buf[1]);
        total_bytes_read++;
        //serial_write_byte(&TIM_SerialUSB, (unsigned char) data_buf[0]);
        data_buf[0] = 0;
        data_buf[1] = 0;
    }

	OS_printf("TIM: Log File size: [%d] bytes.\n", total_bytes_read);


    uint8 file_len[2];
    file_len[0] = *((uint8 *) &total_bytes_read);
    file_len[1] = *(((uint8 *) &total_bytes_read) + 1);
    OS_printf("Image Length MSB = [%#.2X]\n", file_len[1]);
    OS_printf("Image Length LSB = [%#.2X]\n", file_len[0]);

    OS_close(os_fd);
    */



    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xFF);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xF6);
    
    //serial_write_byte(&TIM_SerialUSB, (unsigned char) file_len[1]);
    //serial_write_byte(&TIM_SerialUSB, (unsigned char) file_len[0]); /* check endianess */

    for(index = 0; index < sizeof(log_path); index++)
    {
        serial_write_byte(&TIM_SerialUSB, (unsigned char) logs_filename[index]);
    }

    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x00);

    tim_serial_write_file(&TIM_SerialUSB, (char *) log_path);

    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xFF);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0xF7);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x0D);
    serial_write_byte(&TIM_SerialUSB, (unsigned char) 0x0A);


    CFE_EVS_SendEvent(TIM_COMMAND_LOGS_EID, CFE_EVS_INFORMATION, 
            "Log File \'%s\' Sent Successfully", log_path);

	return;
} /* End of TIM_SendLogFile() */




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* TIM_VerifyCmdLength() -- Verify command packet length                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
boolean TIM_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{
    boolean result = TRUE;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID   = CFE_SB_GetMsgId(msg);
        uint16         CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(TIM_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        TIM_HkTelemetryPkt.tim_command_error_count++;
    }

    return(result);

} /* End of TIM_VerifyCmdLength() */


void TIM_Parallel_Init(void)
{
    wiringPiSetup();
    pinMode(TIM_PAR_GPIO_PIN_PWR, OUTPUT);
    digitalWrite(TIM_PAR_GPIO_PIN_PWR, HIGH);
    CFE_EVS_SendEvent(TIM_PARALLEL_PWR_EID, CFE_EVS_INFORMATION, "Parallel Power Flag Set");
    
    pinMode(TIM_PAR_GPIO_PIN_TE, OUTPUT);
    digitalWrite(TIM_PAR_GPIO_PIN_TE, LOW);
    CFE_EVS_SendEvent(TIM_PARALLEL_TE_EID, CFE_EVS_INFORMATION, "Parallel TE-1 Flag Initialized Low");
    
    /*
    ** Initialize TE-1 digital input with built-in pulldown resistor
    */
    pinMode(TIM_MAIN_GPIO_PIN_TE, INPUT);
    pullUpDnControl(TIM_MAIN_GPIO_PIN_TE, PUD_DOWN); 
    
    if ( digitalRead(TIM_MAIN_GPIO_PIN_TE) == HIGH )
    {
        digitalWrite(TIM_PAR_GPIO_PIN_TE, HIGH);
        CFE_EVS_SendEvent(TIM_PARALLEL_TE_EID, CFE_EVS_INFORMATION, "Parallel TE-1 Flag Set");
    }
    else
    {
        digitalWrite(TIM_PAR_GPIO_PIN_TE, LOW);
        CFE_EVS_SendEvent(TIM_PARALLEL_TE_EID, CFE_EVS_INFORMATION, "Parallel TE-1 Flag Cleared");
    }
        
    return;
}

void TIM_UpdateTimerEvent(void)
{
    if ( digitalRead(TIM_MAIN_GPIO_PIN_TE) == HIGH )
    {
        if( digitalRead(TIM_MAIN_GPIO_PIN_TE) != timer_event_last )
        {
            CFE_EVS_SendEvent(TIM_PARALLEL_TE_EID, CFE_EVS_INFORMATION, "TE-1 UPDATE: New Value");
            digitalWrite(TIM_PAR_GPIO_PIN_TE, HIGH);
            CFE_EVS_SendEvent(TIM_PARALLEL_TE_EID, CFE_EVS_INFORMATION, "TE-1 UPDATE: TE-1 Set");
        }
        timer_event_last = 1;
        
    }
    else
    {
        if( digitalRead(TIM_MAIN_GPIO_PIN_TE) != timer_event_last )
        {
            CFE_EVS_SendEvent(TIM_PARALLEL_TE_EID, CFE_EVS_INFORMATION, "TE-1 UPDATE: New Value");
            digitalWrite(TIM_PAR_GPIO_PIN_TE, LOW);
            CFE_EVS_SendEvent(TIM_PARALLEL_TE_EID, CFE_EVS_INFORMATION, "TE-1 UPDATE: TE-1 Cleared");
        }
        timer_event_last = 0;
        
    }
    return;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                                                          */
/* timer_callback_poweroff_system(uint32 poweroff_timer_id) -- Poweroff the system, called by 				*/
/*															   OSAL Timer at time defined in tim_app.h 	    */
/*                                                                                                          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void timer_callback_poweroff_system(uint32 poweroff_timer_id)
{
	// announce timer to everone logged into the system -- purely for debugging.
	system("wall -n TRAPSat_CFS: timer_callback_poweroff_system called!");

    /*
    ** Send Log files before poweroff from Wallops
    */
    while(serial_busy)
    {
    	// wait until serial not busy
    }
    serial_busy = 1; // Occupy Serial for sending of logs file
    TIM_SendLogFile();
    serial_busy = 0;
    // poweroff the system
    //system("shutdown -P now");
    
    OS_printf("TIM_APP: leaving cFS\n");
    //CFE_PSP_SigintHandler(); // End cFS -- Obviously, this will only happen when Pi is on.
}




