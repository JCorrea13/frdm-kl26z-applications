#include "MKL26Z4.h"
#include "uart0.h"
#include <stdio.h>

void ADC0_Calibrate(void) {
    ADC0_SC3 |= (1U << 7); // Start calibration
    while (!(ADC0_SC1A & ADC_SC1_COCO_MASK)) {
        // Wait until COCO and CAL bits clear automatically, signaling completion
    }
}

void ADC0_IRQHandler(void) {
	// Read ADC0 Result
	uint8_t read_value = (uint8_t)ADC0->R[0];

	//Convert to real value (from 0v to 3.3v)
	float value = (3.3f / 255) * read_value;
	uint8_t integer = (uint8_t)value;
	uint8_t decimal = (uint8_t)((value - integer) * 100);

	// Write value to UART0
	char message[20];
	snprintf(message, sizeof(message), "ADC0 Value: %d.%02d \r\n", integer, decimal);
	UART0_WriteString(message);
}

void PIT_IRQHandler(void) {
	// Validate interrupt was triggered by Channel 0
    if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {

    	// Start next ADC0 conversion in channel 8 and enable interrupt
    	ADC0_SC1A = (8 & ADC_SC1_ADCH_MASK) | ADC_SC1_AIEN_MASK;

    	// Clear the interrupt flag to avoid getting stuck
        PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    }
}

void PIT_Init(void){
	// Enable clock for PIT module
	SIM->SCGC6 |= (1UL << 23);

	// Initialize PIT module
	PIT->MCR = 0x00;

	// Set channel 0 interrupt interval (ticks)
	// Default clock is 4MHz; To trigger the interruption every 500ms we setup to half the timer frequency
	PIT->CHANNEL[0].LDVAL = 2000000 - 1;

	// Configure PIT Channel
	// Bit 1 (TIE) = 1 (Enable Interrupts)
	// Bit 0 (TEN) = 1 (Start the Timer counting down)
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

	// Enable PIT interrupts in the ARM NVIC (Nested Vectored Interrupt Controller)
	NVIC_EnableIRQ(PIT_IRQn);
}

int main(void)
{
	// Init UART
	UART0_Init();

	// Init PIT
	PIT_Init();

	// Enable clock in port B
	SIM->SCGC5 |= (1U << 10);

	// Configure PTB0 Mux Control to alternative 0 (Disabled / Analog pin mode)
	// This detaches the pin from digital GPIO logic to preserve signal integrity
	PORTB->PCR[0] &= ~(7U << 8);

	// Enable system clock for ADC0
	SIM->SCGC6 |= (1U << 27);

	// Select bus clock for ADC0 and Conversion Mode is set to 8-bit single-ended
	ADC0_CFG1 = 0x00U;

	// 5. Clear MUXSEL (Bit 4 = 0) to route channel "a" pins.
	// Channel SE8 on PTB0 is routed natively through the 'a' mux logic
	ADC0_CFG2 &= ~(1U << 4);

	// Selects software trigger
	ADC0_SC2 = 0x00U;

	// Enable ADC0 Interruption
	NVIC_EnableIRQ(ADC0_IRQn);

	// Calibrate ADC0 before first conversion
	ADC0_Calibrate();

	// Infinite Loop
	while(1){
		__asm("NOP");
	}

	return 0;
}
