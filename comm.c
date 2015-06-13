/* Communication Functions */
#include <msp430.h>
#include "defs.h"

void setClocks(int master_frequency, int submain_divider){
	//use Unified Clock System (UCS) module
	//NOTE: REF0 = 32kHz
	
	UCSCTL3 |= SELREF_2; //DCO FLL sourced from REF0
	switch(master_frequency) {
	case MCLK_1MHZ:
	case MCLK_2MHZ:
	case MCLK_4MHZ:
		UCSCTL1 = DCORSEL_4; //DCO frequency range: <6 Mhz
		break;
	case MCLK_8MHZ:
	case MCLK_16MHZ:
		UCSCTL1 = DCORSEL_5; //DCO frequency range: 6 to 23 Mhz
	case MCLK_24MHZ:
		UCSCTL1 = DCORSEL_5; //DCO frequency range: 6 to 23 Mhz
		break;
	}
	UCSCTL4 |= SELA_2; //select REF0 for ACLK -> 32kHz
	
	//disable FLL before modifying it
	__bis_SR_register(SCG0);
	
	//prepare FLL by setting lowest DCO modifier, lowest modulation
	UCSCTL0 = 0x0000;
	
	//set up FLL multiplier
	//FLLN = N = master_frequency, FLLD_0 = D = 1
	//DCOCLKDIV = 1/D * (N+1) * REF0 = 32768 * N * 1/1
	UCSCTL2 = FLLD_0 + master_frequency;
	
	//enable FLL again
	__bic_SR_register(SCG0);
	
	//let FLL settle the clock
	__delay_cycles(60000);
	
	//MCLK divider MCLK = MCLK / 2^DIVM
	UCSCTL5 |= DIVM__1; //set MCLK = 8.388 MHz
	
	//SMCLK divider SMCLK = SMCLK / 2^DIVS
	UCSCTL5 |= submain_divider; //set SMCLK = 8.388 MHz
	
	//let clocks resync (should only take one period)
	__delay_cycles(5);
}

void setupBT2(int clock, int divider_coarse, int divider_fine)
{
	//first stop running module
	UCA1CTL1 |= UCSWRST;
	
	//Using following settings:
	//UCPEN = 0 -> Parity disabled
	//UCPAR = 0 -> Odd parity (if enabled)
	//UCMSB = 0 -> LSB first in TX/RX shift register (little Endian)
	//UC7BIT = 0 -> 8 Data bits
	//UCSPB = 0 -> 1 stop bit
	//UCMODE1 = 00 -> UART mode
	//UCSYNC = 0 -> Asynchronous mode
	UCA1CTL0 = 0x00;
	
	switch(clock) {
	case UCACLK:
		UCA1CTL1 |= UCSSEL_0;
		break;
	case ACLK:
		UCA1CTL1 |= UCSSEL_1;
		break;
	case SMCLK:
		UCA1CTL1 |= UCSSEL_2;
	}
	
	//Set baud rate - see table on page 760 in user guide
	UCA1BR0 = (0x00ff & divider_coarse);
	UCA1BR1 = (0xff00 & divider_coarse) >> 8;
	UCA1MCTLW |= (divider_fine - 1) << 1; //sets UCBRS 
	
	//enable RX interrupt
	UCA0IE |= UCRXIE;
	
	//disable reset pin
	P2SEL &= ~BIT4;
	P2DIR |= BIT4;
	P2OUT |= BIT4;

	//NOTE: UART is held in reset state at this time
}

void startBT2( void ){
	//toggle P1.4 and P1.5 for USCI
	P1SEL |= BIT4 + BIT5;
	
	//release USCI UART from hold state
	UCA1CTL1 &= ~UCSWRST;
}

void stopBT2( void ){
	//stop running module
	UCA1CTL1 |= UCSWRST;
}

void sendData(int data_device, char* data, int packet_size){
	int i = 0;
	switch(data_device) {
	case BT2:
		while (i < packet_size) {
			while (!(UCA1IFG & UCTXIFG));
			UCA1TXBUF = *(data + i);	// send data here...
			i++;
		}
		break;
	}
}
