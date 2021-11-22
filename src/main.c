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

// if defined, debug fail conditions will be printed to the screen upon each failed attempt
#define DEBUG

#ifdef DEBUG
char buff[100];
#endif

void readSensors(ADC_HandleTypeDef adcInstance, uint16_t ADCPressed[]);
void sensorWait(ADC_HandleTypeDef adcInstance, uint16_t ADCPressed[]);

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

    sensorWait(adcInstance, ADCPressed);

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

        // enter loop for current puzzle attempt
        bool failed = false;
        for (int i = 0; !failed; ++i) {
            sensorWait(adcInstance, ADCPressed);

            // determine which sensor was pressed
            // (0 -> yellow, 1 -> green, 2 -> red, 3 -> blue, >3 -> more than one pressed)
            uint16_t pressedSensor = 0;
            for (int i = 0; i < 4; ++i) {
                if (ADCPressed[i]) {
                    pressedSensor += i;
                }
            }

            // evaluate whether sensor pressed matches sequence so far: if so and not at end of sequence, move on;
            // if not, start the next attempt; if so and at end of sequence, begin win routine
            if (pressedSensor == randomLEDSequence[i]) {
                if (i == 3) {
                    complete = true;
                    break;
                }

                // wait for sensor to not be pressed before proceeding on; if another is stepped on before this, attempt is failed
                while(true) {
                    // brief delay so variance in analog value cannot register as an immediate press -> unpress
                    HAL_Delay(100);
                    readSensors(adcInstance, ADCPressed);
                    // proceed if sensor is no longer pressed
                    if (ADCPressed[pressedSensor] == 0) {
                        break;
                    }
                    // tally up pressed values for all sensors -- if greater than 1, more than one sensor is pressed; puzzle failed
                    uint16_t pressCount = 0;
                    for (int j = 0; j < 4; ++j) {
                        if (ADCPressed[j] == 1) { ++pressCount; }
                    }
                    if (pressCount > 1) { 
                        #ifdef DEBUG 
                        sprintf(buff, "Fail condition: Second pad pressed during wait (Tick count: %lu)\n", HAL_GetTick());
                        SerialPuts(buff);
                        sprintf(buff, "Random LED sequence: %hu, %hu, %hu, %hu\n", randomLEDSequence[0], randomLEDSequence[1], randomLEDSequence[2], randomLEDSequence[3]);
                        SerialPuts(buff);
                        sprintf(buff, "Pin A1 (Y): %hu \t Pin A2 (G): %hu \t Pin A3 (R): %hu \t Pin A4 (B): %hu \t pressedSensor = %hu \t i = %hu\n\n", 
                        ReadADC(&adcInstance, ADC_CHANNEL_1), ReadADC(&adcInstance, ADC_CHANNEL_4), ReadADC(&adcInstance, ADC_CHANNEL_8), ReadADC(&adcInstance, ADC_CHANNEL_11), pressedSensor, i);
                        SerialPuts(buff);
                        #endif

                        failed = true; 
                        break;
                    }
                }
                // delay briefly before continuing to ensure small variance in analog values does not immediately fail next part of sequence
                HAL_Delay(100);
                continue;
            } else {
                #ifdef DEBUG 
                char buff[100];
                sprintf(buff, "Fail condition: Either wrong pad pressed or two pressed at once (Tick count: %lu)\n", HAL_GetTick());
                SerialPuts(buff);
                sprintf(buff, "Random LED sequence: %hu, %hu, %hu, %hu\n", randomLEDSequence[0], randomLEDSequence[1], randomLEDSequence[2], randomLEDSequence[3]);
                SerialPuts(buff);
                sprintf(buff, "Pin A1 (Y): %hu \t Pin A2 (G): %hu \t Pin A3 (R): %hu \t Pin A4 (B): %hu \t pressedSensor = %hu \t i = %hu\n\n", 
                ReadADC(&adcInstance, ADC_CHANNEL_1), ReadADC(&adcInstance, ADC_CHANNEL_4), ReadADC(&adcInstance, ADC_CHANNEL_8), ReadADC(&adcInstance, ADC_CHANNEL_11), pressedSensor, i);
                SerialPuts(buff);
                #endif

                failed = true;
            }

        }
    }

    // win routine: blink LEDs quickly thrice
    for (int i = 0; i < 6; ++i) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
        HAL_Delay(500);
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

// waits for a sensor to be pressed
void sensorWait(ADC_HandleTypeDef adcInstance, uint16_t ADCPressed[]) {
    readSensors(adcInstance, ADCPressed);
    while (!(ADCPressed[0] || ADCPressed[1] || ADCPressed[2] || ADCPressed[3])){
        readSensors(adcInstance, ADCPressed);
    }
}

// called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick();  // increment tick count
}