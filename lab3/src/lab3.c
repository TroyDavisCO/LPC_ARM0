/*
===============================================================================
 Name        : lab3.c
 Author      : Troy Davis, Thomas Gayagoy
 Version     : 0.0.1
 Copyright   : None
 Description : This program reads an input waveform, and blinks the on-board led
 based on the signals frequency.  GPIO port 2 is set up as an interrupt driven
 input port.  When GPIO port 2 senses a rising edge on the input signal, the
 current system time from the 32 bit counter is read.  Once a full cycle has
 occurred, the signal frequency will be calculated.  From this frequency calculation
 the 32 bit timer will light the LED.  The system will wake up every 10 ms to
 increment the system time.  Upon wake up, the counter ISR will determine the status
 of the LED.
===============================================================================
*/

/* library includes */
#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif
#include <cr_section_macros.h>
#include <stdio.h>

//globals to calculate frequency and duty cycle for LED
uint32_t timerCount;
uint32_t startTime;
uint32_t stopTime;
uint32_t frequencyHz;

/* GPIO and GPIO Interrupt Initialization */
void GPIOInit() {
	//ENABLE SYSCLK FOR GPIO
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

	//INTERUPT CONFIG
	NVIC_EnableIRQ(EINT2_IRQn);				//enable interrupt for gpio2
	NVIC_SetPriority(EINT2_IRQn,0);			//set interrupt priority
	LPC_GPIO2->IS  &= ~(0x1<<1);			//set interrupt sense
	LPC_GPIO2->IBE &= ~(0x1<<1);			//set edge trigger
	LPC_GPIO2->IEV |=  (0x1<<1);			//interrupt when high
	LPC_GPIO2->IE  |=  (0x1<<1);			//enable interrupt for pin2

	//PIN DIRECTION
	LPC_GPIO2->DIR &= ~(0x1<<1);			//enable GPIO pin2 as input (signal input)
	LPC_GPIO2->DIR |= (0x1<<7);				//enable GPIO pin7 as output (led)

	//VARS FOR INPUT SIGNAL FREQ CALC
	startTime = 0;
	stopTime = 0;
	frequencyHz = 0;
}

/* TIMER32 and TIMER32 Interrupt Initialization */
void TIMERInit() {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10);	//enable clock for timer
	NVIC_EnableIRQ(TIMER_32_0_IRQn);		//enable interrupt for timer
	NVIC_SetPriority(TIMER_32_0_IRQn,0);

	//set variables for timer
	timerCount = 0;
}

/* GPIO Interrupt Handler */
void PIOINT2_IRQHandler() {
	//check if interrupt on pin2 has triggered
	if ( LPC_GPIO2->MIS & (0x1<<1)) {
		if (startTime == 0){
			startTime = timerCount;	//start a new count
		}
		else if (stopTime == 0) {
			stopTime = timerCount;	//stop count
		}
		else {
			uint32_t diff = stopTime - startTime;	//find time diff (increments of 10 ms)
			frequencyHz = 10000/diff;				//calculate frequency (Hz)
			stopTime = startTime = 0;
		}
	}

	//clear the interrupt
	LPC_GPIO2->IC |= (0x1<<1);
}

/* TIMER32 Interrupt Handler */
void TIMER32_0_IRQHandler(){
	//do something
}

int main(void) {

	GPIOInit();		// Initialize GPIO ports for both Interrupts and LED control
	TIMERInit();	// Initialize Timer and Generate a 1ms interrupt

	//do nothing
    while(1);

    return 0;
}



