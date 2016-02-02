/**
 * apmparallel.c
 *
 * Implementation of data transmission procedure.
 *
 * author: Alexander Maricich
 **/
/* Includes */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Constants */
#define BUFF_LEN 4
#define true 1
#define false 0
#define QUEUE_FULL 256
#define QUEUE_PATH "/path/to/the/queue"

/* Prototypes */

/* Main Logic */
int main(int argc, char* argv[])
{
	// Buffer
	/* Logic Implementation */
	// 1) Retrieve queue.
	// 2) Check if the queue is full.
	// 3) If full, pull as much of that data into memory as possible.
	//  :- Loop
	//   4) Pull 4 bits to be transmitted.
	//   5) Wait for the recieved data pulse.
	//   6) Transmit 4 bits segmants of data.
	// 7) If not full, end execution.

	/* Debugging */
	printf("Hello, World");
	return 1;
}

/* Functions */

FILE* retrieve_queue_io()
{
	FILE* queue_file;
	char* mode = "r";

	// Attempt to open the file
	queue_file = fopen(QUEUE_PATH, mode);

	// Check if the file was opened.
	if (queue_file == NULL) {
		fprintf(stderr, "Can't access the queue!");
		exit(1);
	} else
		return queue_file;
}

char* queue_shift(char* queue)
{
	/*return queue<<BUFF_LEN;*/
	return "as";
}

char* read_buffer(char* buffer, FILE* file)
{
	return fgets(buffer, BUFF_LEN, file);
}

int file_size_bytes(FILE* file)
{
	fseek(file, 0L, SEEK_END);
	int file_size = ftell(queue);
	fseek(queue, 0L, SEEK_SET);

	printf("File size: %d bytes/n", file_size);
	return file_size;
}

int queue_full_io()
{
	// Get queue file.
	FILE* queue = retrieve_queue_io();
	int file_size = file_size_bytes(queue);

	// if the queue is ready to be transmitted
	if (file_size >= QUEUE_FULL)
		return true;
	else
		return false;
}
