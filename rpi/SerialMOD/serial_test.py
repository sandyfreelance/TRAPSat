#!/usr/bin/python

import serial
import array
PORT = "/dev/ttyUSB0"
BAUD = 9600#19200
#TIMEOUT = 0.5

data = array.array('B', [0x11]).tostring()

s = serial.Serial( PORT, baudrate=BAUD)

inp = ""
go = 1
while go:
	s.write(data)
	#inp = raw_input("")
	#if(inp == "stop"):
	#	go = 0
s.close()
