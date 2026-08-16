#include "MKL26Z4.h"

void UART0_WriteChar(char c) {
    // Wait while transmit data buffer is full
    while (!(UART0_S1 & (1 << 7))); // Check bit 7 of UART0_S1
    UART0_D = c;                    // Write character to data register
}

void UART0_WriteString(const char *str) {
    while (*str) {
        UART0_WriteChar(*str++);
    }
}

int main(void)
{
	SIM->SCGC5 |= (1UL << 9);  // Enable PORTA clock
	SIM->SCGC4 |= (1UL << 10); // Enable UART clock

	SIM->SOPT2 |= (1 << 26);	// Set internal clock source for UART

	// Configure PORTA 1 and 2 for UART
	// PORTA->PCR[1] = (2UL << 8);  // Set pin 1 to UART Rx (No needed for this exercise)
	PORTA->PCR[2] = (2UL << 8);  // Set pin 2 to UART Tx

	UART0_C2 = 0x00; // Disable UART0 Tx and Rx

	// Configure baud rate to 9600 considering default clock is set to ~20MHz
	// Considering baud rate =  baud clock / ((OSR+1) × BR)
	// BR = 20,971,520Hz/(16 * 9600) = 136.53
	UART0_C4 |= 0x0F; // Set OSR to 15
	UART0_BDH = 0x00; // No need for BDH as BR values is lower than 256
	UART0_BDL = 137;  // Set BDL to closest integer value

	UART0_C2 |= (1UL << 3); // Enable UART0 Tx
	UART0_WriteString("Hello World!");
	UART0_WriteString("Manuel Correa");

	while (1) {}

    return 0;
}
