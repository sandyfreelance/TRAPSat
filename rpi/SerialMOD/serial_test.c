/*
**
** Test of implementing serial_out.h library
**
** compiles with:
**	gcc serial_out.c serial_test.c -o serial_test -lwiringPi
**
*/

#include "serial_out.h"

#define PORT "/dev/ttyUSB0"

int main(void)
{
	serial_out_t serial_usb;

	if(serial_out_init(&serial_usb, (char *)PORT) == -1) // Error opening port
	{
		return -1;
	}
	int i;
	for(i=65;;i++) // infinite loop
	{
		if(i == 91) // set to ascii A->Z
		{
			serial_write_byte(&serial_usb, 10); // write \n
			serial_write_byte(&serial_usb, 13); // write \n	
			i = 65; // reset to A
			
		}
		serial_write_byte(&serial_usb, i); // write data
		printf("serial_usb.data: %x\n", serial_usb.data);
		
	}


	return 0;
}
