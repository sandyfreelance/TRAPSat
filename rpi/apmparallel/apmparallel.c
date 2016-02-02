/**
 * apmparallel.c
 *
 * Implementation of data transmission procedure.
 *
 * author: Alexander Maricich
 **/

#include <stdio.h>
#include <stdint.h>

int main(int argc, char* argv[])
{
	/* Logic Implementation */
	// 1) Retrieve queue.
	// 2) Check if the queue is full.
	// 3) If full, pull as much of that data into memory as possible.
	//  :- Loop
	//   4) Pull 4 bytes to be transmitted.
	//   5) Wait for the recieved data pulse.
	//   6) Transmit 4 byte segmants of data.
	// 7) If not full, end execution.

	/* Debugging */
	printf("Hello, World");
	return 1;
}
