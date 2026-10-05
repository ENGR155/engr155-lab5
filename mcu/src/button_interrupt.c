// button_interrupt.c
// Josh Brake
// jbrake@hmc.edu
// 10/31/22

// Necessary includes for printf to work
#include <stdio.h>
#include "stm32l432xx.h"

// Function used by printf to send characters to the laptop
int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}

///////////////////////////////////////////////////////////////////////////////////////////

#include "main.h"

volatile int interruptcount = 0;

double calcangularvelocity(int count) {
    double vel = 0;

    // If count is 0 - stopped
    if (count == 0) {
        vel = 0;
        printf("Stopped, Not moving\n");
    } else {
        // 120 instead of 408 if ending in -10
        vel = (1.0/(4*408))*count; // Accounts for both positive and negative motion
    }

    return vel;
}

// What is the right name for the IRQHandler? EXTI lines 5-9
/*
Set up a counter variable that counts the number of interrupts
Global count variable resets every second
*/
void EXTI9_5_IRQHandler(void){

    // Check that A triggered an interrupt
    if (EXTI->PR1 & (1 << gpioPinOffset(BUTTON_PIN))){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 = (1 << gpioPinOffset(BUTTON_PIN));

        int a = digitalRead(BUTTON_PIN);
        int b = digitalRead(BUTTON_PIN_2);

        // Counting
        if (a != b) {
            interruptcount++;
        } else {
            interruptcount--;
        }

    }

    // Check that B triggered an interrupt
    if (EXTI->PR1 & (1 << gpioPinOffset(BUTTON_PIN_2))){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 = (1 << gpioPinOffset(BUTTON_PIN_2));

        int a = digitalRead(BUTTON_PIN);
        int b = digitalRead(BUTTON_PIN_2);

        if (a == b) {
            interruptcount++;
        } else {
            interruptcount--;
        }

    }
}



int main(void) {
    // Enable sensor 1 as input
    gpioEnable(GPIO_PORT_A);
    pinMode(BUTTON_PIN, GPIO_INPUT);
    GPIOA->PUPDR &= ~(0b11 << 2*gpioPinOffset(BUTTON_PIN));

    // Enable sensor 2 as an input
    gpioEnable(GPIO_PORT_B);
    pinMode(BUTTON_PIN_2, GPIO_INPUT);
    GPIOA->PUPDR &= ~(0b11 << 2*gpioPinOffset(BUTTON_PIN_2));


    // Initialize timer
    RCC->APB1ENR1 |= (1 << 4); // TIM6EN
    RCC->APB1ENR1 |= (1 << 5); // TIM7EN

    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= (1 << 0);
    // 2. Configure EXTICR for the input button interrupt
    // EXTI7 is bits 14:12 of EXTICR2 (EXTICR[1] in C). Port A is 0b000, so clearing the field selects PA8.
    SYSCFG->EXTICR[1] &= ~(0b1111 << 8);
    SYSCFG->EXTICR[2] &= ~(0b1111 << 0); // Button 2

    // Configure interrupt for rising edge of GPIO pin for button 1
    // 1. Configure mask bit
    // 2. Enable rising edge trigger
    // 3. Enable falling edge trigger
    // 4. Turn on EXTI interrupt in NVIC_ISER
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON_PIN));   // 1. Configure mask bit
    EXTI->RTSR1 |= (1 << gpioPinOffset(BUTTON_PIN)); // 2. Enable rising edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(BUTTON_PIN));  // 3. Enable falling edge trigger
    NVIC->ISER[0] |= (1 << 23);                       // 4. Turn on EXTI interrupt in NVIC_ISER (EXTI9_5 is IRQ 23)

    // Now do the same for button 2
    // 1. Configure mask bit
    // 2. Enable rising edge trigger
    // 3. Enable falling edge trigger
    // Change pin offset and interrupt vector (do somewhere in between 6-9)
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON_PIN_2));   // 1. Configure mask bit
    EXTI->RTSR1 |= (1 << gpioPinOffset(BUTTON_PIN_2)); // 2. Enable rising edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(BUTTON_PIN_2));  // 3. Enable falling edge trigger

    // Enable interrupts globally
    __enable_irq();

    printf("Starting\n");
    initTIM(DELAY_TIM_7);
    initTIM(DELAY_TIM_6);
    while(1){
        // Sets and delays timer 7
        delay_millis(TIM7, 1000);

        /*
        Printing is weird, how to do:
        In SEGGER, right-click your solution name in the left panel.
        Select Options.
        Search for float.
        Set Printf Floating Point Supported to Yes.
        Rebuild your program and restart debugging.
        */

        // Disables the interrupts
        _disable_irq();

        int count2 = interruptcount; // Dummy variable

        interruptcount = 0; // Resets count

        // Enable interrupts globally
        __enable_irq();

        // Prints angular velocity based on dummy timers
        printf("%.3f rev/s\n", calcangularvelocity(count2));

        
    }

}

