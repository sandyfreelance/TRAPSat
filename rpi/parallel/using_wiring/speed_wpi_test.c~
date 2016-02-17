#include <stdio.h>
#include <wiringPi.h>
int GPIO_PIN_BUS[8] = {32, 33, 34, 35, 36, 37, 38, 39}; 
int GPIO_PIN_PRS = 40;
int parallel_write(char data)
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
	return 0;
}

int main(int argc, char* argv[])
{
    unsigned long int debounce;
    wiringPiSetup();
    pinMode(GPIO_PIN_PRS, INPUT);
    
    char data = 0x7E;
    
    int i = 0;
    for(i=0; i < 8; i++)
    {
        pinMode(GPIO_PIN_BUS[i], OUTPUT);
    }
    
    printf("Pins Initialized Correctly\n");
    
    for(;;)
    {
        if( digitalRead(GPIO_PIN_PRS) == HIGH )
        {
            debounce += 1;
            if((debounce % 2) == 0)
            {
                parallel_write(data);
            }
            else
            {
                parallel_write(0x00);
            }
        }
        else if(debounce != 0)
        {
            printf("debounce reached: %d\n", debounce);
            debounce = 0;
            printf("debounce set to zero\n");
        }
    }
    return 0;
}
