/*******************************************************************************
** File: vc0706_device.c
**
** Purpose:
**   This is a source file for the VC0706 application
**
**   int VC0706_takePics(void) "VC0706 Camera capture Loop"
**
**
*******************************************************************************/
/*
** VC0706 Master Header
*/
#include "vc0706.h"


/*
** Local Functions
*/
char * getTime(void);


/*
** External References 
*/
extern struct led_t led;
extern struct mux_t mux;
extern struct Camera_t cam;

/*
** VC0706 take Pictures Loop
*/
int VC0706_takePics(void)
{
    /*
    ** Path that pictures should be stored in
    **
    ** NOTE: if path is greater than 16 chars, imageName[] in vc0706_core.h will need to be enlarged accordingly.
    */
    char * path = malloc(OS_MAX_PATH_LEN);

    /*
    ** Attempt to initialize LED
    */ 
    if(led_init(&led, LED_PIN) == -1)
    {
        OS_printf("LED initialization error.\n");
        return -1;
    }

    /*
    ** Initialize MUX
    */
    if(mux_init(&mux, MUX_SEL_PIN) == -1)
    {
        OS_printf("MUX initialization error.\n");
        return -1;
    }

    /*
    ** Attempt to initalize Camera
    */
    if(init(&cam) == -1) // Error
    {
        OS_printf("Camera initialization error.\n");
        return -1;
    }

    /*
    ** infinite Camera loop
    ** w/ no delay
    */
    int i;
    for (i=0; ;i++) // NOTE: we will need to add flash and MUX implementation. Easy, but should be broken into separate headers.
    {
	   char *v;

        /*
        ** Get camera version, another way to check that the camera is working properly. Also necessary for initialization.
        */
        if ((v = getVersion(&cam)) == NULL) // function will return NULL upon failure
        {
            OS_printf( "Failed communication to Camera.\n"); // maybe more information would be good, we'll see what's relevant durring testing.
            return -1;
        }
        /*else // pulled to limit debug prints
        {
            OS_printf("Debug: Camera open with version = %s\n", v);
        }*/

        /*
        ** Set Path for the new image
        */
	    OS_printf("VC0706: Calling sprintf()...\n");
	    char * num_reboots = "und"; // initialized to undefined
        //num_reboots = getNumReboots(); // not writen yet -- ask Keegan. 
        int ret = sprintf(path, "/home/pi/TRAPSat/images/%s_%d.jpg", num_reboots, i);
    	if(ret < 0)
    	{
    	    OS_printf("sprintf err: %s\n", strerror(ret));
    	}

        /*
        ** Switch Cameras -- Has not been tested with hardware yet
        */
        mux_switch(&mux);

    	/*
        ** Actually takes the picture
        */
    	OS_printf("VC0706: Calling takePicture()...\n");
        char* loc = takePicture(&cam, path);
        OS_printf("Debug: Camera took picture. Stored at: %s\n", loc);

    	} /* Infinite Camera capture Loop End Here */

        return(0);
}


/*char * getTime(void)
{
    char * curr_time;
    time_t raw_t;
    time(&raw_t);
    struct tm * timeinfo;
    timeinfo = localtime( &raw_t );
    //printf("%s\n", asctime(timeinfo));

    //sprintf(curr_time, "%s", (char[])asctime( timeinfo ) );
    curr_time = asctime(timeinfo);
    char * i;
    for(i=curr_time; *i ; i++)
    {
        if( *i == ' ')
            *i = '_';
        if( *i == ':')
            *i = '-';
	if( *i == '\n')
	    *i = '\0';
    }

    return curr_time;
}*/
