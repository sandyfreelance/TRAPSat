/*
** This is going to be a long night.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <unistd.h> /* Unix standard function definitions */
#include <fcntl.h> /* File control definitions */
#include <errno.h> /* Error number definitons */
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "ads1115_device.h"


int rpicm_i2c_detect_bus()
{
	int fd;
	
	if ((fd = open("/dev/i2c-0", O_RDWR)) != -1)
	{
		close(fd);
		return 0;
	}
	else if ((fd = open("/dev/i2c-1", O_RDWR))  != -1)
	{
		close(fd);
		return 1;
	}
	else if ((fd = open("/dev/i2c-2", O_RDWR))  != -1)
	{
		close(fd);
		return 2;
	}

	/*
	** Could not find I2C bus
	*/
	close(fd);
	return -1;
}

int rpicm_i2c_open_bus(int i2c_bus)
{
	if (i2c_bus < 0 || i2c_bus > 2)
	{
		fprintf(stderr, "rpicm_i2c_open_bus(int) recieved unexpected arg (%d)\n", i2c_bus);
		return -1;
	}
	
	//printf("debug: inside int rpicm_i2c_open_bus(int) - passed arg error\n");

	int fd;
	char filename[12];
	char i2c_bus_id = i2c_bus + '0'; /* convert to ascii*/
	
	//printf("debug: inside int rpicm_i2c_open_bus(int) - passed convert\n");

	if ((snprintf(filename, sizeof(filename), "/dev/i2c-%c", i2c_bus_id)) < 0)
	{
		fprintf(stderr, "rpicm_i2c_open_bus(int) snprintf error\n");
		return -1;
	}
	
	//printf("debug: inside int rpicm_i2c_open_bus(int) - passed concat\n");
	
	if ((fd = open(filename, O_RDWR)) < 0)
	{
		fprintf(stderr, "rpicm_i2c_open_bus(int) failed to open %s\n", filename);
		return -1;
	}
	
	//printf("debug: inside int rpicm_i2c_open_bus(int) - passed open file\n");
	
	/*
	** Return successfully opened file descriptor
	*/
	return fd;
}

#define ADS1115_ADDR  0b01001001                                    /* (0x49) */
#define ADS1115_CONV_ADDR 0b00000000                         /* (0x00) */
#define ADS1115_CFG_ADDR 0b00000001                            /* (0x01) */

#define ADS1115_POST_DATA_CMD        0b10010001         /* (0x91) */
#define ADS1115_LISTEN_CMD                0b10010000         /* (0x90) */

#define ADS1115_CFG_DEFAULT_MSB    0x85
#define ADS1115_CFG_DEFAULT_LSB    0x83


int main(int argc, char *argv[])
{
    int bus_id;
    int i2c_fd;
    int io_res;
    //uint8_t i2c_cfg_data[3] = {ADS1115_CFG_ADDR, ADS1115_CFG_DEFAULT_MSB, ADS1115_CFG_DEFAULT_LSB};
    uint8_t i2c_cfg_data[3] = {ADS1115_CFG_DEFAULT_LSB, ADS1115_CFG_DEFAULT_MSB, ADS1115_CFG_ADDR};
    uint8_t i2c_data[2] = {0x00, 0x00};
    
    if ((bus_id = rpicm_i2c_detect_bus()) < 0)
    {
        fprintf(stderr, "error: rpicm_i2c_detect_bus() = %d\n", bus_id);
        exit(-1);
    }
    else
    {
	    printf("rpicm_i2c_detect_bus() = %d\n", bus_id);
	}
	
	if ((i2c_fd = rpicm_i2c_open_bus(bus_id)) < 0)
	{
	    fprintf(stderr, "error: rpicm_i2c_open_bus(%d) = %d\n", bus_id, i2c_fd);
	    exit(-1);
	}
	else
	{
    	printf("rpicm_i2c_open_bus(%d) = %d\n", bus_id, i2c_fd); 
	}

	/*
	** Address the I2C device
	*/
    if ((io_res = ioctl(i2c_fd, I2C_SLAVE, ADS1115_ADDR)) < 0)
    {
        fprintf(stderr, "failed control io device, io_res = %d\n", io_res);
        exit(-1);
    }
    

    /*
    ** Configure I2C device
    */
     if ((io_res = i2c_smbus_write_byte(i2c_fd, ADS1115_LISTEN_CMD)) < 0)
    {
        fprintf(stderr, "cfg step 1 failed writing to i2c device, io_res = %d\n", io_res);
    }
    else
    {
         if ((io_res = i2c_smbus_write_byte(i2c_fd, ADS1115_CFG_ADDR)) < 0)
         {
            fprintf(stderr, "failed writing CFG_ADDR to i2c device, io_res = %d\n", io_res);
         }
         if ((io_res = i2c_smbus_write_byte(i2c_fd, ADS1115_CFG_DEFAULT_MSB)) < 0)
         {
            fprintf(stderr, "failed writing CFG_MSB to i2c device, io_res = %d\n", io_res);
         }
         if ((io_res = i2c_smbus_write_byte(i2c_fd, ADS1115_CFG_DEFAULT_LSB)) < 0)
         {
            fprintf(stderr, "failed writing CFG_LSB to i2c device, io_res = %d\n", io_res);
         }
        
        if ((io_res = i2c_smbus_write_byte_data(i2c_fd, ADS1115_LISTEN_CMD, ADS1115_CONV_ADDR)) < 0)
        {
            fprintf(stderr, "cfg step 2 failed writing to i2c device, io_res = %d\n", io_res);
        }
    }

    /*
    ** Read loop
    */
	while(1)
    {    
        /*
        ** Send I2C device post data command
        */
        i2c_data[1] = 0x00;
        i2c_data[0] = 0x00;
        if ( (io_res = i2c_smbus_read_i2c_block_data(i2c_fd, ADS1115_POST_DATA_CMD, 2, i2c_data)) < 0)
        {
            fprintf(stderr, "Failed reading data from i2c device\n");
        }
        else
        {
            printf("Data[0]: %#2.2X \t %d\n", i2c_data[0], i2c_data[0]);
            printf("Data[1]: %#2.2X \t %d\n", i2c_data[1], i2c_data[1]);
            printf("Hex Data: %#2.2X\n", i2c_data);
            int data = i2c_data[1] << 8 | i2c_data[0];
            printf("Dec Data: %d\n", data);
        }
        /*
        printf("\n");
        i2c_data[1] = 0x00;
        i2c_data[0] = 0x00;
        
        if ((io_res = i2c_smbus_write_byte(i2c_fd, ADS1115_POST_DATA_CMD)) < 0)
        {
            fprintf(stderr, "failed writing to i2c device, io_res = %d\n", io_res);
        }
        else
        {
            i2c_data[1] = 0;
            i2c_data[0] = 0;
            i2c_data[1] = i2c_smbus_read_byte(i2c_fd);
            i2c_data[0] = i2c_smbus_read_byte(i2c_fd);
            printf("Hex Data: %#4.4X\n", i2c_data);
            int data;
            data = i2c_data[1] << 8 | i2c_data[0];
            printf("Dec Data: %d\n", data);
        }
        */
        sleep(1);
    }
	return 0;
}



/*
** End of file.
*/