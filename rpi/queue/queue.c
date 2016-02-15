#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define true 1
#define false 0
#define QUEUE_SIZE 10000 // bytes
#define DELIMITER 'G'
#define FILE_DATA 1
#define TEMP_DATA 2

/*Any functions with a suffix of '_io' has some sort of IO associated with it.*/
/*These will most likely take more time to complete and should be used sparringly.*/
uint8_t* create_queue(int size_in_bytes);
void delete_queue(uint8_t* queue_array);
int file_size_bytes_io(char* file_path);
int insert_byte(uint8_t* queue, int offset, uint8_t byte);
int insert_int(uint8_t* queue, int offset, int integer);
int queue_add_file_io(uint8_t* queue, int offset, char* file_path);


int main(int argc, char* argv[])
{

	uint8_t* queue;
	int bytes_filled = 0;
	int transmit_ready = false;

	// Create the queue
	queue = create_queue(QUEUE_SIZE);

	bytes_filled += queue_add_file_io(queue, bytes_filled, "/home/amaricich/Pictures/noodlebird.png");
	bytes_filled += queue_add_file_io(queue, bytes_filled, "/home/amaricich/Pictures/noodlebird.png");
	printf("%c\n", queue[0]);
	printf("%i\n", queue[1]);
	printf("%d\n", queue[2]);
	printf("%d\n", queue[3]);
	printf("%d\n", queue[4]);
	printf("%c\n", queue[5]);

	/*void queue_add_temperature(queue, bytes_filled,"This is some temperature data: 1.22348347238");*/
	// Pull in image data
	/*int data_size = file_size_bytes_io("/home/amaricich/Pictures/noodlebird.png");*/
	// Store image in the queue
	// if the queue is full start transmission

	// kill the queue.
	delete_queue(queue);
	return 1;
}

/* Functions */

/*char* queue_shift(char* queue)*/
/*{*/
	/*[>return queue<<BUFF_LEN;<]*/
	/*return "as";*/
/*}*/

/*Create the queue*/
uint8_t* create_queue(int size_in_bytes)
{
	uint8_t* queue_array;
	queue_array = (uint8_t*)calloc(size_in_bytes, sizeof(uint8_t));
	return queue_array;
}

/*Clear the queue*/
void delete_queue(uint8_t* queue_array)
{
	free(queue_array);
}

/*Retrieve size of file in bytes*/
int file_size_bytes_io(char* file_path)
{
	FILE* f;
	char* mode = "r";

	// Attempt to open the file
	f = fopen(file_path, mode);

	// Check if the file was opened.
	if (f == NULL) {
		fprintf(stderr, "Could not open file at path %s.\n", file_path);
		exit(1);
	} else {
		fseek(f, 0L, SEEK_END);
		int file_size = ftell(f);
		fseek(f, 0L, SEEK_SET);

		fclose(f);
		printf("File size: %d bytes\n", file_size);
		return file_size;
	}
}

int insert_byte(uint8_t* queue, int offset, uint8_t byte)
{
	queue[offset] = byte;
	return sizeof(uint8_t);
}

int insert_int(uint8_t* queue, int offset, int integer)
{
	int internal_offset = offset;
	insert_byte(queue, internal_offset++, (integer >> (8*0)) & 0xff);
	insert_byte(queue, internal_offset++, (integer >> (8*1)) & 0xff);
	insert_byte(queue, internal_offset++, (integer >> (8*2)) & 0xff);
	insert_byte(queue, internal_offset++, (integer >> (8*3)) & 0xff);
	return sizeof(int);
}

int queue_add_file_io(uint8_t* queue, int offset, char* file_path)
{
	FILE* f;
	char* mode = "r";
	int file_size;

	/*Open file*/
	f = fopen(file_path, mode);

	if (f == NULL) {
		fprintf(stderr, "Could not open file at path %s.\n", file_path);
		exit(1);
	} else {
		/*Get the size of the file*/
		fseek(f, 0L, SEEK_END);
		file_size = ftell(f);
		fseek(f, 0L, SEEK_SET);
		printf("********************\n");
		printf("File size: %d bytes\n", file_size);

		/*Read the entire file into the queue if there is space*/

		printf("offset: %d bytes\n", offset);
		printf("combined: %d bytes\n", file_size+offset);
		printf("queue size: %d bytes\n", QUEUE_SIZE);
		if (file_size+offset < QUEUE_SIZE)
		{
			printf("Possible to add file to queue.\n");
			int internal_offset = offset;
			/*Insert starting character and size*/
			internal_offset += insert_byte(queue, internal_offset,(uint8_t)DELIMITER);
			internal_offset += insert_int(queue, internal_offset, file_size);

			/*Close the file*/
			fclose(f);
			return internal_offset - offset;
		}
		else
		{
			printf("Impossible to add file to queue.\n");

			/*Close the file*/
			fclose(f);
			return 0;
		}
	}


}

/*void queue_add_string(char* string)*/
/*{*/
/*}*/

/*void queue_add_temperature(uint8_t* queue, int offset, char* temp_data)*/
/*{*/
/*}*/
