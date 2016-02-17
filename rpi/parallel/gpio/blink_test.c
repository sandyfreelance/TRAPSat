/*
http://www.pieter-jan.com/node/15
2013 Pieter-Jan Van de Maele All Rights Reserved
*/
#include "rpi_gpio_periph.h"
 
extern struct bcm2835_peripheral gpio = {GPIO_BASE};


int main(int argc, char* argv[])
{
     if(map_peripheral(&gpio) == -1) 
    {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        unmap_peripheral(&gpio);
        return -1;
    }
    
    int gpio_pin = 0;
    
    for(gpio_pin=42; gpio_pin <= 45; gpio_pin++)
    {
        INP_GPIO(gpio_pin);
        OUT_GPIO(gpio_pin);
    }    

    /*
    ** Infinite Flash Loop
    */
    while(1)
    {
        /*
        for(gpio_pin=24; gpio_pin <= 31; gpio_pin++)
        {
            GPIO_SET0 |= 1 << (gpio_pin);
        }
        */
        for(gpio_pin=42; gpio_pin <= 45; gpio_pin++)
        {
            GPIO_SET1 |= 1 << (gpio_pin - 32);
        }
        
        sleep(3);
        /*
        for(gpio_pin=24; gpio_pin < 32; gpio_pin++)
        {
            GPIO_CLR0 |= 1 << (gpio_pin);
        }
        */
        for(gpio_pin=42; gpio_pin < 45; gpio_pin++)
        {
            GPIO_CLR1 |= 1 << (gpio_pin -32);
        }
        
        sleep(3);
    }
    
    unmap_peripheral(&gpio);
    
	return 0;
}
