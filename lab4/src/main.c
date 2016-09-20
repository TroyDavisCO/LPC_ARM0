/*
===============================================================================
 Name        : main.c
 Author      : Troy Davis, Thomas Gayagoy
 Version     : 0.0.1
 Copyright   : None
 Description : Main program for lab4
===============================================================================
*/

#define LPCXPRESSO_LPC1343_BOARD
#include "driver_config.h"
#include "target_config.h"
#include "timer16.h"
#include "wakeupdefs.h"

int main(void) {

	/* Do nothing - Go to sleep to save power between interrupts */
	while(1){
	}

    return 0;
}
