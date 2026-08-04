#include "MKL26Z4.h"

void delay(){
	uint32_t counter = 2000000;
	while (counter--){
		__asm("NOP"); // Prevents the compiler from optimizing out the empty loop
	}
}

int main(void)
{
	// Enables clock for Port D for Blue led
	SIM->SCGC5 |= (1UL << 12);

	// Configure blue pin (PTD5) for led as GPIO mode
	PORTD->PCR[5]  = (1UL << 8);

	// Set pin (PTD5) data direction to output
	PTD->PDDR |= (1UL << 5); // Output mode for Blue

	// Blink
	while (1) {
		// Turn ON
		PTD->PCOR = (1UL << 5);
		delay();
		// Turn OFF
		PTD->PSOR = (1UL << 5);
		delay();
	}

	return 0;
}
