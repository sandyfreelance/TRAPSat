/*
 * Retrieved from https://github.com/vyykn/VC0706
 *
 * Edited By Zach Richard for use on TRAPSat aboard the RockSat-X 2016 Mission
 */



#include <time.h> // #include <ctime>
#include <errno.h>
#include <stdbool.h>

#include <stdint.h>
#include <stdlib.h>

// Brian's DEPS
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <wiringSerial.h>
#include <wiringPi.h>

// Brian's Defines
#define BAUD 38400

#define RESET 0x26
#define GEN_VERSION 0x11
#define READ_FBUF 0x32
#define GET_FBUF_LEN 0x34
#define FBUF_CTRL 0x36
#define DOWNSIZE_CTRL 0x54
#define DOWNSIZE_STATUS 0x55
#define READ_DATA 0x30
#define WRITE_DATA 0x31
#define COMM_MOTION_CTRL 0x37
#define COMM_MOTION_STATUS 0x38
#define COMM_MOTION_DETECTED 0x39
#define MOTION_CTRL 0x42
#define MOTION_STATUS 0x43
#define TVOUT_CTRL 0x44
#define OSD_ADD_CHAR 0x45

#define STOPCURRENTFRAME 0x0
#define STOPNEXTFRAME 0x1
#define RESUMEFRAME 0x3
#define STEPFRAME 0x2

#define SIZE640 0x00
#define SIZE320 0x11
#define SIZE160 0x22

#define MOTIONCONTROL 0x0
#define UARTMOTION 0x01
#define ACTIVATEMOTION 0x01

#define SET_ZOOM 0x52
#define GET_ZOOM 0x53

#define CAMERABUFFSIZ 100
#define CAMERADELAY 10

#define TO_SCALE 1
#define TO_U 200000



typedef struct Camera {
    int motion;
    int ready;
    int fd;
    uint8_t offscreen[8]; // font width;

    int frameptr;
    int bufferLen;
    int serialNum;
    char camerabuff[CAMERABUFFSIZ+1];
    char serialHeader[5];
    char imageName[16];
    char * empty;
} Camera;



void init(Camera *cam) {
    cam->frameptr = 0;
    cam->bufferLen = 0;
    cam->serialNum = 0;
    cam->motion = 1;
    cam->ready = 1;

    if ((cam->fd = serialOpen("/dev/ttyAMA0", BAUD)) < 0)
        fprintf(stderr, "SPI Setup Failed: %s\n", strerror(errno));

    if (wiringPiSetup() == -1)
        exit(1);

    cam->ready = 1;
}


void reset(Camera *cam) {
    // Camera Reset method
    serialPutchar(cam->fd, (char)0x56);
    serialPutchar(cam->fd, (char)cam->serialNum);
    serialPutchar(cam->fd, (char)RESET);
    serialPutchar(cam->fd, (char)0x00);

    if (checkReply(RESET, 5) != true)
        fprintf(stderr, "Check Reply Status: %s\n", strerror(errno));

    clearBuffer(*cam);
}

bool checkReply(Camera *cam, int cmd, int size) {
    int reply[size];
    int t_count = 0;
    int length = 0;
    int avail = 0;
    int timeout = 3 * TO_SCALE; // test 3 was 5
    
    while ((timeout != t_count) && (length != CAMERABUFFSIZ) && length < size)
    {
        avail = serialDataAvail(cam->fd);
        if (avail <= 0)
        {
            usleep(TO_U);
            t_count++;
            continue;
        }
        t_count = 0;
        // there's a byte!
        int newChar = serialGetchar(cam->fd);
        reply[length++] = (char)newChar;
    }
    
    //Check the reply
    if (reply[0] != 0x76 && reply[1] != 0x00 && reply[2] != cmd)
        return false;
    else
        return true;
}

void clearBuffer(Camera *cam) {
    int t_count = 0;
    int length = 0;
    int timeout = 2 * TO_SCALE;

    while ((timeout != t_count) && (length != CAMERABUFFSIZ))
    {
        int avail = serialDataAvail(cam->fd);
        if (avail <= 0)
        {
            t_count++;
            continue;
        }
        t_count = 0;
        // there's a byte!
        serialGetchar(cam->fd);
        length++;
    }
}

void resumeVideo(Camera *cam)
{
    serialPutchar(cam->fd, (char)0x56);
    serialPutchar(cam->fd, (char)cam->serialNum);
    serialPutchar(cam->fd, (char)FBUF_CTRL);
    serialPutchar(cam->fd, (char)0x01);
    serialPutchar(cam->fd, (char)RESUMEFRAME);
    
    if (checkReply(cam, FBUF_CTRL, 5) == false)
        printf("Camera did not resume\n");
}

char * getVersion(Camera *cam)
{
    serialPutchar(cam->fd, (char)0x56);
    serialPutchar(cam->fd, (char)cam->serialNum);
    serialPutchar(cam->fd, (char)GEN_VERSION);
    serialPutchar(cam->fd, (char)0x00);
    
    if (checkReply(cam, GEN_VERSION, 5) == false)
    {
        printf("CAMERA NOT FOUND!!!\n");
    }
    
    int counter = 0;
    cam->bufferLen = 0;
    int avail = 0;
    int timeout = 1 * TO_SCALE;
    
    while ((timeout != counter) && (bufferLen != CAMERABUFFSIZ))
    {
        avail = serialDataAvail(cam->fd);
        if (avail <= 0)
        {
            usleep(TO_U);
            counter++;
            continue;
        }
        counter = 0;
        // there's a byte!
        int newChar = serialGetchar(cam->fd);
        cam->camerabuff[cam->bufferLen++] = (char)newChar;
    }
    
    cam->camerabuff[cam->bufferLen] = 0;
    
    return camerabuff;
}

void setMotionDetect(Camera *cam, int flag)
{
    serialPutchar(cam->fd, (char)0x56);
    serialPutchar(cam->fd, (char)0x00);
    serialPutchar(cam->fd, (char)0x42);
    serialPutchar(cam->fd, (char)0x04);
    serialPutchar(cam->fd, (char)0x01);
    serialPutchar(cam->fd, (char)0x01);
    serialPutchar(cam->fd, (char)0x00);
    serialPutchar(cam->fd, (char)0x00);
    
    serialPutchar(cam->fd, (char)0x56);
    serialPutchar(cam->fd, (char)cam->serialNum);
    serialPutchar(cam->fd, (char)COMM_MOTION_CTRL);
    serialPutchar(cam->fd, (char)0x01);
    serialPutchar(cam->fd, (char)flag);

    clearBuffer(cam);
}


char * takePicture(Camera *cam, const char * file_path)
{
    cam->frameptr = 0;

    // Force Stop motion detect
    //setMotionDetect(0);

    //Clear Buffer
    clearBuffer(cam);

    serialPutchar(cam->fd, (char)0x56);
    serialPutchar(cam->fd, (char)cam->serialNum);
    serialPutchar(cam->fd, (char)FBUF_CTRL);
    serialPutchar(cam->fd, (char)0x01);
    serialPutchar(cam->fd, (char)STOPCURRENTFRAME);
    
    if (checkReply(cam, FBUF_CTRL, 5) == false)
    {
        printf("Frame checkReply Failed\n");
        return empty;
    }

    
    serialPutchar(cam->fd, (char)0x56);
    serialPutchar(cam->fd, (char)cam->serialNum);
    serialPutchar(cam->fd, (char)GET_FBUF_LEN);
    serialPutchar(cam->fd, (char)0x01);
    serialPutchar(cam->fd, (char)0x00);
    
    if (checkReply(cam, GET_FBUF_LEN, 5) == false)
    {
        printf("FBUF_LEN REPLY NOT VALID!!!\n");
        return empty;
    }
    
    while(serialDataAvail(cam->fd) <= 0){}

    printf("Serial Data Avail %u \n", serialDataAvail(cam->fd));
    
    int len;
    len = serialGetchar(cam->fd);
    len <<= 8;
    len |= serialGetchar(cam->fd);
    len <<= 8;
    len |= serialGetchar(cam->fd);
    len <<= 8;
    len |= serialGetchar(cam->fd);

    printf("Length %u \n", len);

    if(len > 20000){
        printf("To Large... \n");
        resumeVideo(cam);
        clearBuffer(cam);
        return takePicture(cam, file_path);
    }
    
    char image[len];
    
    int imgIndex = 0;
    
    while (len > 0)
    {
        int readBytes = len;
        
        serialPutchar(cam->fd, (char)0x56);
        serialPutchar(cam->fd, (char)cam->serialNum);
        serialPutchar(cam->fd, (char)READ_FBUF);
        serialPutchar(cam->fd, (char)0x0C);
        serialPutchar(cam->fd, (char)0x0);
        serialPutchar(cam->fd, (char)0x0A);
        serialPutchar(cam->fd, (char)(cam->frameptr >> 24 & 0xff));
        serialPutchar(cam->fd, (char)(cam->frameptr >> 16 & 0xff));
        serialPutchar(cam->fd, (char)(cam->frameptr >> 8 & 0xff));
        serialPutchar(cam->fd, (char)(cam->frameptr & 0xFF));
        serialPutchar(cam->fd, (char)(readBytes >> 24 & 0xff));
        serialPutchar(cam->fd, (char)(readBytes >> 16 & 0xff));
        serialPutchar(cam->fd, (char)(readBytes >> 8 & 0xff));
        serialPutchar(cam->fd, (char)(readBytes & 0xFF));
        serialPutchar(cam->fd, (char)(CAMERADELAY >> 8));
        serialPutchar(cam->fd, (char)(CAMERADELAY & 0xFF));
            
        if (checkReply(cam, READ_FBUF, 5) == false)
        {
            return empty;
        }
        
        int counter = 0;
        cam->bufferLen = 0;
        int avail = 0;
        int timeout = 20 * TO_SCALE;
        
        while ((timeout != counter) && cam->bufferLen < readBytes)
        {
            avail = serialDataAvail(cam->fd);

            if (avail <= 0)
            {
                usleep(TO_U);
                counter++;
                continue;
            }
            counter = 0;
            int newChar = serialGetchar(cam->fd);
            image[imgIndex++] = (char)newChar;
            
            cam->bufferLen++;
        }
        
        cam->frameptr += readBytes;
        len -= readBytes;
        
        if (checkReply(cam, READ_FBUF, 5) == false)
        {
            printf("ERROR READING END OF CHUNK| start: %u | length: %u\n", frameptr, len);
        }
    }
       
    FILE *jpg = fopen(file_path, "w");
    if (jpg != NULL)
    {
        fwrite(image, sizeof(char), sizeof(image), jpg);
        fclose(jpg);
    }
    else
    {
        printf("IMAGE COULD NOT BE OPENED/MADE!\n");
    }
    
    sprintf(imageName, "%s", file_path);
    
    resumeVideo(cam);

    //Clear Buffer
    clearBuffer(cam);

    // Force Stop motion detect
    //setMotionDetect(0);

    
    return imageName;
}

// End Brian's Methods

// There once was a bunch of Node JS code here .... it is gone now -- Zach
