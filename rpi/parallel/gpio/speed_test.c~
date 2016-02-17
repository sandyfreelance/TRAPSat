/*
http://www.pieter-jan.com/node/15
2013 Pieter-Jan Van de Maele All Rights Reserved
*/
#include "rpi_gpio_periph.h"
 
extern struct bcm2835_peripheral gpio = {GPIO_BASE};
int GPIO_PIN_BUS[8] = {32, 33, 34, 35, 36, 37, 38, 39}; 
int GPIO_PIN_PRS = 40;

/*
** This function currently assumes
*/
int parallel_write(char data)
{  

    int gpio_pin = 0;
    int i = 0;
    
    for(i=0; i < 8; i++)
    {
        gpio_pin = GPIO_PIN_BUS[i];

        if ( data & (1 << i) )
        {
            /*
            ** Set pins for output
            */
            INP_GPIO(gpio_pin);
            OUT_GPIO(gpio_pin);
            GPIO_SET1 |= 1 << (gpio_pin - 31);
        }
        else
        {
            INP_GPIO(gpio_pin);
        }
    }
       
	return 0;
}

int main(int argc, char* argv[])
{
     if(map_peripheral(&gpio) == -1) 
    {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        unmap_peripheral(&gpio);
        return -1;
    }
    
    INP_GPIO(GPIO_PIN_PRS);
    
    //char flag = 0xDD;  
    char flag = 0x7E; // '~' = 0x7E
    char data = 0;
    int debouncer = 0;
    
    /*
    ** Debug
    */
    //printf("test1\n");
    printf("GPIO_READ1(GPIO_PIN_PRS - 32): %d\n", GPIO_READ1(GPIO_PIN_PRS - 32));
    //printf("test2\n");
    /*
    
    while( 1 )
    {
        if ( GPIO_READ1(GPIO_PIN_PRS - 32)  )
        {
            printf("PRS Hit!\n");
            debouncer++;
        }
        else if ( debouncer != 0 )
        {
            printf("debounce reached: %d\n", debouncer);
            debouncer = 0;  
            printf("debounce set to 0.\n");
        }
        else
        {
            debouncer = 0;
        }
        //printf("In the Loop.\n");
    }
    */    
    unmap_peripheral(&gpio);
    
    return 0;
}
