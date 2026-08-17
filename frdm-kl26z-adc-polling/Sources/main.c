#include "MKL26Z4.h"
#include "uart0.h"
#include <stdio.h>

void ADC0_Calibrate(void) {
    ADC0_SC3 |= (1U << 7); // Start calibration
    while (!(ADC0_SC1A & ADC_SC1_COCO_MASK)) {
        // Wait until COCO and CAL bits clear automatically, signaling completion
    }
}

uint8_t ADC0_ReadChannel(uint8_t channel) {
    // Triggering a software translation is achieved by writing the target channel code to SC1A
    // Writing to SC1A clears any active translation context and launches a new sequence
    ADC0_SC1A = (channel & ADC_SC1_ADCH_MASK);

    // Block actively by tracking the Conversion Complete Flag (COCO)
    while (!(ADC0_SC1A & ADC_SC1_COCO_MASK)) {
        // Spin lock wait cycle
    }

    // Reading the result register R0 drops the COCO indicator flag automatically
    return (uint8_t)ADC0->R[0];
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

	// Calibrate ADC0 before first conversion
	ADC0_Calibrate();

	uint8_t read_value = 0;
	char message[20];

	// Infinite loop
	while (1) {
		// Read directly from Analog Channel 8 (ADC0_SE8 maps to PTB0)
		read_value = ADC0_ReadChannel(8);

		//Convert to real value (from 0v to 3.3v)
		float value = (3.3f / 255) * read_value;
		uint8_t integer = (uint8_t)value;
		uint8_t decimal = (uint8_t)((value - integer) * 100);

		// Write value to UART0
		snprintf(message, sizeof(message), "ADC0 Value: %d.%02d \r\n", integer, decimal);
		UART0_WriteString(message);

		// delay
		for (volatile int i = 0; i < 2000000; i++);
	}

	 return 0;
}
