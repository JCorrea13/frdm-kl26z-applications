#include "MKL26Z4.h"
#include "uart0.h"
#include <stdio.h>

volatile uint8_t adc_read_completed = 0;

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

	adc_read_completed = 1;
}

int main(void)
{
	// Init UART
	UART0_Init();

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

	// Start the first conversion
	ADC0_SC1A = (8 & ADC_SC1_ADCH_MASK) | ADC_SC1_AIEN_MASK;

	while(1) {

		if(adc_read_completed){
			// reset flag
			adc_read_completed = 0;

			// delay
			for (volatile int i = 0; i < 2000000; i++);

			// start next conversion in channel 8
			ADC0_SC1A = (8 & ADC_SC1_ADCH_MASK) | ADC_SC1_AIEN_MASK;
		}

	}

	return 0;
}
