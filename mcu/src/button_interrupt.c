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

int main(void) {
    // Enable LED as output
    gpioEnable(GPIO_PORT_B);
    pinMode(LED_PIN, GPIO_OUTPUT);

    // Enable button as input
    gpioEnable(GPIO_PORT_A);
    pinMode(BUTTON_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= (0b01 << 2*gpioPinOffset(BUTTON_PIN)); // Set PA7 as pull-up (PUPD7 = 01)

    // Initialize timer
    RCC->APB1ENR1 |= (1 << 0); // TIM2EN
    initTIM(DELAY_TIM);

    // TODO
    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= (1 << 0);
    // 2. Configure EXTICR for the input button interrupt
    // EXTI7 is bits 14:12 of EXTICR2 (EXTICR[1] in C). Port A is 0b000, so clearing the field selects PA7.
    SYSCFG->EXTICR[1] &= ~(0b111 << 12);


    // Enable interrupts globally
    __enable_irq();

    // TODO: Configure interrupt for falling edge of GPIO pin for button 1
    // 1. Configure mask bit
    // 2. Enable rising edge trigger
    // 3. Disable falling edge trigger
    // 4. Turn on EXTI interrupt in NVIC_ISER
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON_PIN));   // 1. Configure mask bit
    EXTI->RTSR1 |= (1 << gpioPinOffset(BUTTON_PIN)); // 2. Enable rising edge trigger
    EXTI->FTSR1 &= ~(1 << gpioPinOffset(BUTTON_PIN));  // 3. Disable falling edge trigger
    NVIC->ISER[0] |= (1 << 23);                       // 4. Turn on EXTI interrupt in NVIC_ISER (EXTI9_5 is IRQ 23)

    // Now do the same for button 2
    // 1. Configure mask bit
    // 2. Enable rising edge trigger
    // 3. Disable falling edge trigger

    // TODO: Change pin offset and interrupt vector (do somewhere in between 6-9)
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON_PIN_2));   // 1. Configure mask bit
    EXTI->RTSR1 |= (1 << gpioPinOffset(BUTTON_PIN_2)); // 2. Enable rising edge trigger
    EXTI->FTSR1 &= ~(1 << gpioPinOffset(BUTTON_PIN_2));  // 3. Disable falling edge trigger

    // Button 1 press
    void EXTI2_IRQHandler(void) {
    if (EXTI->PR1 & (1 << 2)) {
        EXTI->PR1 |= (1 << 2); // Clear interrupt flag
        // Handle Button 1 press here
        printf("Button 1 Pressed");
    }


    while(1){   
        delay_millis(TIM2, 200);
    }

}

// TODO: What is the right name for the IRQHandler? EXTI lines 5-9
void XXXXXX(void){
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << )){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 = (1 << );

        // Then toggle the LED
        togglePin(LED_PIN);

    }
}
