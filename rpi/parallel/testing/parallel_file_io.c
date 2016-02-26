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

int parallel_write(unsigned char data);

int GPIO_PIN_BUS[8] = {32, 33, 34, 35, 36, 37, 38, 39}; 
int GPIO_PIN_PRS = 40;

int main(int argc, char* argv[])
{
    int final_state = 0;
    int src_state = 0;
    int send_state = 0;
    unsigned long int debounce;
    unsigned char data = START_MAIN_FLAG;
    unsigned char src_data_buf = 0x00;
    int ret = 0;
    int i = 0;
    int file_length = 0;
    unsigned char file_length_msb, file_length_lsb;
    struct stat st;
    int src_data_file;
    wiringPiSetup();
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
        parallel_write(0x00);
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
    sleep(1);
    
    /*
    ** Write Loop
    */
    for(;;)
    {
        if( (digitalRead(GPIO_PIN_PRS) == HIGH)  && (debounce == 0) )
        {
            parallel_write(data);
            debounce = 1;
        }
        else if( (digitalRead(GPIO_PIN_PRS) == HIGH)  && (debounce >= 1) )
        {
            debounce += 1;
        }
        else if ( debounce >= 1 )
        {
            /*
            ** Delay 5 micro seconds
            ** This allows microcontroller to read data
            ** may slow it down TOO much
            */
            usleep(5);

            /*
            ** get new data
            */
            switch(send_state)
            {
                case 0: 
                    /*
                    ** Send State: START NULL BYTE
                    */
                    data = 0x00;
                    send_state = 1;
                    break;
                case 1: 
                    /*
                    ** Send State: START FLAG
                    */
                    data = START_MAIN_FLAG;
                    send_state = 2;
                    break;
                case 2: 
                    /*
                    ** Send State: DATA ID
                    */
                    switch(src_state)
                    {
                        case SRC_PICT_ST:
                            data = START_PICT_FLAG;
                            break;
                        case SRC_TEMP_ST:
                            data = START_TEMP_FLAG;
                            break;
                        default: data = START_OTHR_FLAG;
                    }
                    send_state = 3;
                    break;
                case 3: 
                    /*
                    ** Send State: START SIZE MSB
                    */
                    data = file_length_msb;
                    send_state = 4;
                    break;
                case 4: 
                    /*
                    ** Send State: START SIZE LSB
                    */
                    data = file_length_lsb;
                    send_state = 5;
                    break;
                case 5: 
                    /*
                    ** Send State: START NULL BYTE
                    */
                    data = 0x00;
                    send_state = 6;
                    break;
                case 6:
                    /*
                    ** Send State: Update Data from Source 
                    */
                    if((ret = read(src_data_file, &src_data_buf, 1)) == 0)
                    {
                        printf("Final State:%d\n", final_state);
                        
                        switch(final_state)
                        {
                            case 0:
                                data = 0x00;
                                final_state = 1;
                                break;
                            case 1:
                                data = START_MAIN_FLAG;
                                final_state = 2;
                                break;
                            case 2:
                                data = START_STOP_FLAG;
                                final_state = 3;
                                break;
                            case 3:
                                data = 0x00;
                                final_state = 4;
                                break;
                case 4: 
                    /*
                    ** Send State: START SIZE LSB
                    */
                    data = file_length_lsb;
                    send_state = 5;
                    break;
                case 5: 
                    /*
                    ** Send State: START NULL BYTE
                    */
                    data = 0x00;
                    send_state = 6;
                    break;
                            case 4:
                                /*
                                ** Reset States
                                */
                                final_state = 0;
                                send_state = 0;
                                close(src_data_file);
                                return 0;
                                break;
                            default:
                                final_state = 0;
                                data = 0x00;
                        }
                    }
                    else
                    {
                        data = src_data_buf;
                    }
                    break;
                default: 
                    data = 0x00;
            }

            //printf("debounce reached: %d\n", debounce);
            debounce = 0;
        }
    }

    close(src_data_file);

    return 0;
}

int parallel_write(unsigned char data)
{
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
    //printf("Byte Data Written: %X\n", data);
	return 0;
}
