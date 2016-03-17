#include "serial_out.h"

int serial_out_init(serial_out_t *serial, char * port) // opens the serial port and sets it to serial->fd
{
	serial->fd = serialOpen(port, BAUD);
	serial->data = 0x00;
	if(serial->fd == -1)
	{
		OS_printf("serial_out: ERROR: %s", strerr(errno));
		return -1;
	}
	return 0;
}

void write_byte(serial_out_t *serial, unsigned char byte) // writes byte to serial port
{
	serialPutchar(serial->fd, byte);
	serial->data = byte;
}
