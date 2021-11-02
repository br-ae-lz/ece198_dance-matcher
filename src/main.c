// Modified sample code for ECE 198

// Written by Bernie Roehl, August 2021
// Modified by Braden Schulz, October 2021

#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions

#include "ece198.h"

int main(void)
{
    HAL_Init(); // initialize the Hardware Abstraction Layer

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)

    // turn on the LEDs wired up to D12 and GND by setting up D12 for output
    InitializePin(GPIOA, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);

    // toggle D12 every 125 ms, switching power on and off to blink the LEDs
    while(1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
        HAL_Delay(125);
    }

    return 0;
}

// This function is called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
}
