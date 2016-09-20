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
#define TIMER_INTERVAL	(SystemCoreClock/1000 - 1)  //1 ms interval
#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led
#define ON 1
#define OFF 0

//globals to calculate frequency and duty cycle for LED sdf
uint32_t timerCount;
uint32_t startTime;
uint32_t stopTime;

uint32_t ledTimer;
uint32_t led25Percent;
uint32_t led75Percent;
uint8_t ledStatus;


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
	LPC_GPIO2->IE  |=  (0x1<<1);			//enable interrupt for pin1

	//PIN DIRECTION
	LPC_GPIO2->DIR &= ~(0x1<<1);			//enable GPIO pin2 as input (signal input)
	LPC_GPIO[LED_PORT]->DIR |= (0x1<<LED_BIT);	//enable GPIO pin7 as output (led)

	//VARS FOR INPUT SIGNAL FREQ CALC
	startTime = 0;
	stopTime = 0;

	led25Percent = 0;
	led75Percent = 0;
	ledTimer = 0;
	ledStatus = OFF;
}


/* TIMER32 and TIMER32 Interrupt Initialization */
void TIMERInit() {
	//set variables for timer
	timerCount = 0;
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);	//enable clock for timer

	LPC_TMR32B0->MCR = 3;   //match control register 0b11, enables and resets
	LPC_TMR32B0->MR0 = 10000; //timer value for interrupt, 1 ms interval
    LPC_TMR32B0->CCR=0; //disable capture
    LPC_TMR32B0->PR=0; // count every prescale pulse
    LPC_TMR32B0->PC=0; // initial prescale count

	NVIC_EnableIRQ(TIMER_32_0_IRQn);		//enable interrupt for timer
	NVIC_SetPriority(TIMER_32_0_IRQn,0);
	LPC_TMR32B0->TCR = 2;
	LPC_TMR32B0->TCR = 1;  // enable timers
}

/* GPIO Interrupt Handler */
void PIOINT2_IRQHandler() {
	//check if interrupt on pin2 has triggered
	if ( LPC_GPIO2->MIS & (0x1<<1)) {
		if (startTime == 0) {
			startTime = timerCount;	//start a new count
			//LPC_GPIO2->MASKED_ACCESS[(1<<7)] = (ON<<7);
		}
		else if (stopTime == 0) {
			stopTime = timerCount;	//stop count
			//LPC_GPIO2->MASKED_ACCESS[(1<<7)] = (OFF<<7);
		}
		else {
			uint32_t diff = stopTime - startTime;		//find time diff (increments of 10 ms)
			led25Percent = diff * 0.25;
			led75Percent = diff * 0.75;
			stopTime = startTime = 0;				//reset time for next period calculation
		}
		//clear the interrupt
		LPC_GPIO2->IC |= (0x1<<1);
	}
	return;
}

/* TIMER32 Interrupt Handler */
void TIMER32_0_IRQHandler(){
	if ( LPC_TMR32B0->IR & 0x01 )
	  {
		LPC_TMR32B0->IR = 1;				/* clear interrupt flag */
		timerCount++;
	  }

	if(timerCount < 10000)    // before 10 seconds
	{
		if (ledStatus == OFF) {
			if (ledTimer == 0) {
				ledTimer = led25Percent;
				//turn on led
				LPC_GPIO[LED_PORT]->MASKED_ACCESS[(1<<LED_BIT)] = (ON<<LED_BIT);
			}
			else {
				ledTimer--;
			}
		}
		else if (ledStatus == ON) {
			if (ledTimer == 0) {
				ledTimer = led25Percent;
				//turn off led
				LPC_GPIO[LED_PORT]->MASKED_ACCESS[(1<<LED_BIT)] = (OFF<<LED_BIT);
			}
			else {
				ledTimer--;
			}
		}
	}

    if(timerCount >= 10000)    // after 10 seconds
    {
    	if (ledStatus == OFF) {
    		if (ledTimer == 0) {
    			ledTimer = led75Percent;
    			//turn on led
    			LPC_GPIO[LED_PORT]->MASKED_ACCESS[(1<<LED_BIT)] = (ON<<LED_BIT);
    		}
    		else {
    			ledTimer--;
    		}
    	}
    	else if (ledStatus == ON) {
    		if (ledTimer == 0) {
    			ledTimer = led75Percent;
    			//turn off led
    			LPC_GPIO[LED_PORT]->MASKED_ACCESS[(1<<LED_BIT)] = (OFF<<LED_BIT);
    		}
    		else {
    			ledTimer--;
    		}
    	}
    }
}


int main(void) {

	GPIOInit();		// Initialize GPIO ports for both Interrupts and LED control
	TIMERInit();	// Initialize Timer and Generate a 1ms interrupt


	//do nothing
    while(1);

    return 0;
}



