/*
**
** Original source samples copied from:
** http://chrisheydrick.com/2012/06/17/how-to-read-serial-data-from-an-arduino-in-linux-with-c-part-3/
**
*/

#include <stdio.h>
#include <stdint.h> /* uint8_t definintion */
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

#define SERIAL_BUF_SIZE 6
#define START_OTHR_FLAG 0x21 /* '!' */
#define START_TEMP_FLAG 0x24 /* '$' */
#define START_PICT_FLAG 0x7E /* '~' */ 
#define START_STOP_FLAG 0x3F /* '?' */
#define START_MAIN_FLAG 0x40 /* '@' */


int main()
{
    /*********************************************************************************************/
    char *portname = "/dev/ttyUSB0";
    int fd;
     
    /* 
    ** Open the file descriptor in non-blocking mode 
    */
    fd = open(portname, O_RDWR | O_NOCTTY);
     
    /*
    ** Set up the control structure 
    */
    struct termios toptions;
     
    /*
    ** Get currently set options for the tty
    */
    tcgetattr(fd, &toptions);
     
    /* Set custom options */
     
    /*
    ** 9600 baud = B9600 
    ** 115200 baud = B115200
    */
    cfsetispeed(&toptions, B115200);
    cfsetospeed(&toptions, B115200);
    
    /* 
    ** 8 bits, no parity, no stop bits 
    */
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    
    /*
    ** no hardware flow control
    */
    toptions.c_cflag &= ~CRTSCTS;
    
    /*
    ** enable receiver, ignore status lines 
    */
    toptions.c_cflag |= CREAD | CLOCAL;
    
    /* 
    ** disable input/output flow control, disable restart chars 
    */
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    /* 
    ** disable canonical input, disable echo,
    ** disable visually erase chars,
    ** disable terminal-generated signals 
    */
    toptions.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /*
    ** disable output processing
    */
    toptions.c_oflag &= ~OPOST;
     
    /* 
    ** wait for 12 characters to come in before read returns 
    ** WARNING! THIS CAUSES THE read() TO BLOCK UNTIL ALL 
    ** CHARACTERS HAVE COME IN! 
    */
    toptions.c_cc[VMIN] = 1;
    
    /* 
    ** no minimum time to wait before read returns 
    */
    toptions.c_cc[VTIME] = 0;
     
    /* 
    ** commit the options 
    */
    tcsetattr(fd, TCSANOW, &toptions);
     
    /********************** LOOK DOWN HERE *****************/
    /* 
    ** Wait for the Arduino to reset 
    */
    usleep(1000*1000);
    /* 
    ** Flush anything already in the serial buffer 
    */
    tcflush(fd, TCIFLUSH);
    /********************** LOOK UP HERE ********************/
    /*********************************************************************************************/

    unsigned char serial_buf[SERIAL_BUF_SIZE];
    memset(serial_buf, '\0', sizeof(serial_buf));
    unsigned char serial_byte_buf = '\0';
    unsigned short file_size = 0;
    int file_state = 0;
    int read_count = 0;
    int data_count = 0;

    FILE *data_file = fopen("/home/keegan/Arduino/TRAPSat/parallel/prs_data_relay/test/data.jpeg", "wb+");

    while(1)
    {
        read_count = read(fd, &serial_byte_buf, 1);
        //printf("n = %d\n", n);
        data_count += read_count;

        if(read_count > 0)
        {
            printf("\tHex Data: %X\n", serial_buf[0]);
            /*
            if(serial_buf[0] != 0)
            {
                printf("\tHex Data: %X\n", serial_buf[0]);
                //printf("\tDec Data:%u\n", serial_buf[0]);
            }
            */

            /*
            ** Shift Flag Buffer before inserting new byte
            */
            serial_buf[5] = serial_buf[4];
            serial_buf[4] = serial_buf[3];
            serial_buf[3] = serial_buf[2];
            serial_buf[2] = serial_buf[1];
            serial_buf[1] = serial_buf[0];
            serial_buf[0] = serial_byte_buf;


            if  (
                    (serial_buf[0] == 0x00) && 
                    (serial_buf[4] == START_MAIN_FLAG) && 
                    (serial_buf[5] == 0x00)
                )
            {
                printf("Found Start/Stop Flags: {0,1,2,3,4,5}={%X, %X, %X, %X, %X, %X}\n", 
                    serial_buf[0], serial_buf[1], serial_buf[2], serial_buf[3], serial_buf[4], serial_buf[5]);

                switch(serial_buf[3])
                {
                    case START_TEMP_FLAG:
                        file_state = 1;
                        break;
                    case START_PICT_FLAG:
                        file_state = 2;
                        break;
                    case START_OTHR_FLAG:
                        file_state = 3;
                        break;
                    case START_STOP_FLAG:
                        file_state = 0;
                        /*
                        ** Reached End Of Transmission
                        */
                        close(data_file);
                        printf("Serial Collect finished.\n");
                        printf("Ending Start/Stop Flags: {0,1,2,3,4,5}={%X, %X, %X, %X, %X, %X}\n", 
                            serial_buf[0], serial_buf[1], serial_buf[2], serial_buf[3], serial_buf[4], serial_buf[5]);
                        printf("Serial Collect finished. Data Count=(%u) File Size=(%u) File State=(%d)\n", data_count, file_size, file_state);
                        return 0;
                        break;
                    default:
                        //printf("default found\n");
                        break;
                }

                if(file_state > 0 && file_state < 4)
                {
                    printf("Data Count has reached: %d\n", data_count);
                    data_count = 0;

                    /*
                    ** Get Size Data
                    */
                    file_size = (serial_buf[2] << 8) + serial_buf[1];
                    printf("start flag found! Size = %u\n", file_size);
                    file_state = 4;
                }
            }
            /*
            else if ((data_count >= file_size + 4) && file_state > 0)
            {
                read_count = read(fd, &serial_buf[0], 4);
                if  (
                        (serial_buf[0] == 0x00) && 
                        (serial_buf[1] == START_STOP_FLAG) && 
                        (serial_buf[2] == START_MAIN_FLAG) && 
                        (serial_buf[3] == 0x00) 
                    )
                {
                    printf("Received Stop Flag\n");
                }
                close(data_file);
                printf("Ending Start/Stop Flags: {0,1,2,3,4,5}={%X, %X, %X, %X, %X, %X}\n", 
                    serial_buf[0], serial_buf[1], serial_buf[2], serial_buf[3], serial_buf[4], serial_buf[5]);
                printf("Serial Collect finished. Data Count=(%u) File Size=(%u) File State=(%d)\n", data_count, file_size, file_state);
                return 0;
            }
            */
            
            if(file_state == 4)
            {
                printf("attempting to write to file (%X)\n", serial_byte_buf);
                write(data_file, &serial_byte_buf, 1);
                fwrite(&serial_byte_buf, 1, 1, data_file);
                //fprintf(data_file, "%X", serial_byte_buf);
            }
        }
    }

    printf("Serial Collect finished.\n");

    return 0;
}