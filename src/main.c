// ECE 198 Pressure Sensor LED Pattern Project
// Written by Braden Schulz, November 2021

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> 

#include "ece198.h"

// define minimum ADC values for each sensor that indicate they are being pressed
#define A1_PRESSED 3000
#define A2_PRESSED 3000
#define A3_PRESSED 3000
#define A4_PRESSED 3000

void readSensors(ADC_HandleTypeDef adcInstance, uint16_t ADCPressed[]);

int main(void)
{
    // initialize HAL and enable ports to be used
    HAL_Init();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

    // initalize D12, D11, D10, and D9 for output
    // (correspond to yellow, green, red, and blue LEDs respectively)
    InitializePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
    InitializePin(GPIOB, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
    InitializePin(GPIOC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
    // initialize A1, A2, A3, and A4 for analog input
    // (correspond to pressure sensors for yellow, green, red, and blue LEDs respectively)
    InitializePin(GPIOA, GPIO_PIN_1 | GPIO_PIN_4, GPIO_MODE_ANALOG, GPIO_NOPULL, 0);
    InitializePin(GPIOB, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_NOPULL, 0);   
    InitializePin(GPIOC, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_NOPULL, 0); 

    // set up serial communication
    SerialSetup(9600);

    // declare and initialize ADC instance
    ADC_HandleTypeDef adcInstance;
    InitializeADC(&adcInstance, ADC1);

    // declare analog input array to store pressed state for pins A1-A4
    uint16_t ADCPressed[4];

    // wait for a sensor to be pressed
    readSensors(adcInstance, ADCPressed);
    while (!(ADCPressed[0] || ADCPressed[1] || ADCPressed[2] || ADCPressed[3])){
        readSensors(adcInstance, ADCPressed);
    }

    // enter main puzzle loop
    bool complete = false;
    while (!complete)
    {
        for (int i = 0; i < 2; ++i) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7);
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
            HAL_Delay(1000);
        }

        // compose a random sequence of 4 LEDs and briefly flash them accordingly
        // (0 -> yellow, 1 -> green, 2 -> red, 3 -> blue)
        uint16_t randomLEDSequence[4];
        srand(HAL_GetTick());
        for (int i = 0; i < 4; ++i) {
            randomLEDSequence[i] = (rand() % 4);

            switch (randomLEDSequence[i]) 
            {
                case 0:
                    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
                    break;
                case 1:
                    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
                    break;
                case 2:
                    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
                    break;
                case 3:
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
                    break;
            }
            HAL_Delay(125);
            switch (randomLEDSequence[i]) 
            {
                case 0:
                    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
                    break;
                case 1:
                    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
                    break;
                case 2:
                    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
                    break;
                case 3:
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
                    break;
            }
            HAL_Delay(500);
        }
        
    }

    return 0;
}

// reads pressure sensors and updates their pressed states
void readSensors(ADC_HandleTypeDef adcInstance, uint16_t ADCPressed[]) {
    if (ReadADC(&adcInstance, ADC_CHANNEL_1) > A1_PRESSED) {
        ADCPressed[0] = 1;
    } else {
        ADCPressed[0] = 0;
    }
    if (ReadADC(&adcInstance, ADC_CHANNEL_4) > A2_PRESSED) {
        ADCPressed[1] = 1;
    } else {
        ADCPressed[1] = 0;
    }
    if (ReadADC(&adcInstance, ADC_CHANNEL_8) > A3_PRESSED) {
        ADCPressed[2] = 1;
    } else {
        ADCPressed[2] = 0;
    }
    if (ReadADC(&adcInstance, ADC_CHANNEL_11) > A4_PRESSED) {
        ADCPressed[3] = 1;
    } else {
        ADCPressed[3] = 0;
    }
}

// called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick();  // increment tick count
}