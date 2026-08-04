#include "MKL26Z4.h"

// Variable to signal the timer has been trigger
// Volatile forces both routines, the main program and the interrupt function to always read/write from/to the main memory avoiding reading outdated values.
volatile uint8_t timer_triggered = 0;

// Interrupt Service Routine for PIT
void PIT_IRQHandler(void) {
	// Validate interrupt was triggered by Channel 0
    if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
        timer_triggered = 1;

        // Clear the interrupt flag to avoid getting stuck
        PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    }
}

int main(void)
{
	// Enable Clock for Port E (Red led) and PIT module
	SIM->SCGC5 |= (1UL << 13);  // Port E
	SIM->SCGC6 |= (1UL << 23);  // PIT Clock Gate

	// Configure Red led (PTE29), set GPIO mode and output direction
	PORTE->PCR[29] = (1UL << 8);
	PTE->PDDR |= (1UL << 29);

	// Initialize PIT module
	PIT->MCR = 0x00;

	// Set channel 0 interrupt interval (ticks)
	// Default clock is 4MHz; To trigger the interruption every 500ms we setup to half the timer frequency
	PIT->CHANNEL[0].LDVAL = 2000000;

	// Configure PIT Channel
	// Bit 1 (TIE) = 1 (Enable Interrupts)
	// Bit 0 (TEN) = 1 (Start the Timer counting down)
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

	// Enable PIT interrupts in the ARM NVIC (Nested Vectored Interrupt Controller)
	NVIC_EnableIRQ(PIT_IRQn);

	while (1) {
		if (timer_triggered) {
			timer_triggered = 0;
			PTE->PTOR = (1UL << 29); // Toggle the LED
		}
		__asm("WFI"); // Sleep CPU while waiting for the PIT interrupt
	}

	return 0;
}
