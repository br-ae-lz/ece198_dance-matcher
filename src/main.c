// Modified sample code for ECE 198

// Written by Bernie Roehl, August 2021
// Modified by Braden Schulz, November 2021

#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions

#include "ece198.h"

int main(void)
{
    HAL_Init(); // initialize the Hardware Abstraction Layer

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable port B (for the rotary encoder inputs, for example)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable port C (for the on-board blue pushbutton, for example)

    InitializePin(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // on-board LED

    // set up for serial communication to the host computer
    // (anything we write to the serial port will appear in the terminal (i.e. serial monitor) in VSCode)

    SerialSetup(9600);

    __HAL_RCC_ADC1_CLK_ENABLE();        // enable ADC 1
    ADC_HandleTypeDef adcInstance;      // this variable stores an instance of the ADC
    InitializeADC(&adcInstance, ADC1);  // initialize the ADC instance

    // initialize temporary values to store min and max readings and whether input is read
    uint16_t maxraw0 = 0;
    uint16_t minraw0 = 10000;
    bool pressed = false;

    // Enables the input pins
    // (on this board, pin A0 is connected to channel 0 of ADC1, and A1 is connected to channel 1 of ADC1)
    InitializePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_NOPULL, 0);   
    while (true)
    {
        // read the ADC values (0 -> 0V, 2^12 -> 3.3V)
        uint16_t raw0 = ReadADC(&adcInstance, ADC_CHANNEL_0);

        // update min and max values if readings lie outside these limits
        if (maxraw0 < raw0) {
            maxraw0 = raw0;
        } if (minraw0 > raw0) {
            minraw0 = raw0;
        } 
        
        // reading under 70 is read as a press, so print whether sensor is being pressed based off of that
        if (raw0 < 70) {
            pressed = true;
        } else {
            pressed = false;
        }

        // print the ADC values, min and max values read so far, and whether sensor is being pressed
        char buff[100];
        sprintf(buff, "Channel0: %hu, max: %hu, min: %hu, pressed: %d\r\n", raw0, maxraw0, minraw0, pressed);  // hu == "unsigned short" (16 bit)
        SerialPuts(buff);
    }

    return 0;
}

// This function is called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
}
