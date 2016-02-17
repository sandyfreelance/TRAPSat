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
    /*
    ** Set pins for output
    */
    int gpio_pin = 0;
      
    /*
    for(gpio_pin= gpio_lsb; gpio_pin < gpio_msb; gpio_pin++)
    {
        INP_GPIO(gpio_pin);
        OUT_GPIO(gpio_pin);
    } 
    */
    
    int i = 0;
    for(i=0; i < 8; i++)
    {
        gpio_pin = GPIO_PIN_BUS[i];

        if ( data & (1 << i) )
        {
            INP_GPIO(gpio_pin);
            OUT_GPIO(gpio_pin);
            
            //printf("Setting pin %d\n", gpio_pin);
            //sleep(2);
            GPIO_SET1 |= 1 << (gpio_pin - 31);
            //sleep(1);
        }
        else
        {
            //printf("Clearing pin %d\n", gpio_pin);
            //sleep(2);
            INP_GPIO(gpio_pin);
            //sleep(1);
            
            /*
            sleep(2);
            GPIO_CLR1 |= 1 << (gpio_pin -32);
            sleep(1);
            */
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
    
    printf("test1\n");
    printf("GPIO_READ1(GPIO_PIN_PRS - 32): %d\n", GPIO_READ1(GPIO_PIN_PRS - 32));
    printf("test2\n");
    
    while( 1 )
    {
        if ( GPIO_READ1(GPIO_PIN_PRS - 32) /*&& (debouncer == 0) */ )
        {
            printf("PRS hit!\n");
            
            if (data == flag)
            {
                data = 0;
                printf("OFF - %c\n", data);
            }
            else
            {
                data = flag;
                printf("ON - %c\n", data);
            }
            
            parallel_write(data);
            
            debouncer = 1;
            
        }
        /*
        else if ( (GPIO_READ1(GPIO_PIN_PRS - 32) == 0) && (debouncer == 1) )
        {
            debouncer = 0;            
            printf("debounce set to 0.\n");
        }
        */
        //printf("In the Loop.\n");
    }
        
    unmap_peripheral(&gpio);
    
    return 0;
}
