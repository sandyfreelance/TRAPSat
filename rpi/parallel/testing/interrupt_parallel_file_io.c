/*
 * This is the base code to be used for the Parallel Telemetry lines.
 */

#include <stdio.h>
#include <fcntl.h>
#include <wiringPi.h>

#define SRC_UNKNOWN_ST 0
#define SRC_TEMP_ST 1
#define SRC_PICT_ST 2

#define START_MAIN_FLAG 0x40 /* '@' */
#define START_STOP_FLAG 0x3F /* '?' */
#define START_PICT_FLAG 0x7E /* '~' */
#define START_TEMP_FLAG 0x24 /* '$' */
#define START_OTHR_FLAG 0x21 /* '!' */

void parallel_write(void);

unsigned char data;
unsigned long int debounce = 0;


int t0, tf;

//RPi CM
int GPIO_PIN_BUS[8] = {32, 33, 34, 35, 36, 37, 38, 39};
int GPIO_PIN_PRS = 40;

// RPi B+
//int GPIO_PIN_BUS[8] = {18, 23, 24, 25, 8, 7, 12, 16};
//int GPIO_PIN_PRS = 02;


int src_data_file;
int ret = 1; // needs to be initialized to not 0


int main(int argc, char* argv[])
{
    t0=0;
    tf=0;

    int final_state = 0;
    int src_state = 0;
    int send_state = 0;
    data = START_MAIN_FLAG;
    int i = 0;
    int file_length = 0;
    unsigned char file_length_msb, file_length_lsb;
    unsigned char src_data_buf = 0x00;
    struct stat st;


    //RPi CM
	wiringPiSetup();

    //RPi B+
	//wiringPiSetupGpio();

    pinMode(GPIO_PIN_PRS, INPUT);

    /*
    ** Determine Source and set src_state accordingly
    */
    src_data_file = open(argv[1], O_RDONLY);
    stat(argv[1], &st);
    file_length = st.st_size;
    file_length_msb = (unsigned char) ((file_length & 0xFF00) >> 8);
    file_length_lsb = (unsigned char) (file_length & 0x00FF);
    //printf("MSB=%X\n", file_length_msb);
    //printf("LSB=%X\n", file_length_lsb);
    //printf("opening %s = %d\t size = %d\n", argv[1], src_data_file, file_length);
    src_state = SRC_PICT_ST;

    if((ret = read(src_data_file, &src_data_buf, 1)) == 0)
    {
        printf("Failed to get source data.\n");
        //parallel_write(0x00);
        data = 0x00;
	close(src_data_file);
        return 0;
    }

    /*
    ** Set Parallel Pins to Output using WiringPi 
    */
    for(i=0; i < 8; i++)
    {
        pinMode(GPIO_PIN_BUS[i], OUTPUT);
    }
    printf("Pins Initialized Correctly\n");
    //sleep(1);
    delayMicroseconds(1);


    /*
    ** Set interrupt on PRS pin for parallel_write() -------------------------- New addition in functionality
    */
    wiringPiISR( GPIO_PIN_PRS, INT_EDGE_RISING, &parallel_write );


    for(;;) { // wait for IO to complete
	if(ret == 0) {
	    break;
	}
    }

    close(src_data_file);

    return 0;
}

void parallel_write(void) // now uses global data variable
{
    t0 = micros();

    delayMicroseconds(5);

    int gpio_pin = 0;
    int i = 0;
    for(i=0; i < 8; i++)
    {
        gpio_pin = GPIO_PIN_BUS[i];

        if ( data & (1 << i) )
        {
            digitalWrite(gpio_pin, HIGH);
        }
        else
        {
            digitalWrite(gpio_pin, LOW);
        }
    }

    // get new data
    ret = read(src_data_file, &data, 1);

    tf = micros() - t0;
    if(tf < 100) {
        delayMicroseconds(100-tf);
    }
}
