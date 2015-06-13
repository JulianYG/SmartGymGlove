#include "msp430.h"
#include "defs.h"
#include "mode_turbine.h"
#include "comm.h"

#define LIPOLY
#define DO_NOTHING &no_operation_func

void (*F_init)();  //sensor function pointers
void (*F_run)();
void (*F_stop)();
void (*F_ta3_cc)();
void (*F_save)();

int boardstatus;
void no_operation_func(void){} //this function is a placeholder for empty function pointers//this function is a placeholder for empty function pointers
void sleep(void);

void turnOff(void){

	boardstatus = 0; //OFF

	sleep();

	P1SEL = P2SEL = P3SEL = P4SEL = P5SEL = P6SEL = P7SEL = P8SEL = PJSEL = P9SEL = 0;
	P1DIR = P2DIR = P3DIR = P4DIR = P5DIR = P6DIR = P7DIR = PJDIR = P9DIR = 0;
    P8DIR = BIT1 ;
    P1REN = P2REN = P3REN = P4REN = P5REN = P6REN = P7REN = P8REN = PJREN = P9REN = 0xFF;
    P1OUT = P2OUT = P3OUT = P4OUT = P5OUT = P6OUT = P7OUT = PJOUT = P9OUT = 0xFF;
    P8OUT = ~BIT1;

    TA2CCTL0 &= ~CCIE;
    TA2CCTL1 &= ~CCIE;
    TA3CCTL0 &= ~CCIE;

	P2REN |= BIT6;                          // Enable P2.6 internal resistance
	P2OUT &= ~BIT6;                          // Set P2.6 as pull-down resistance
    P2IE |= BIT6;                           // P2.6 interrupt enabled
    P2IES &= ~BIT6;                          // P2.6 low->high edge
    P2IFG &= ~BIT6;                         // P2.6 IFG cleared

    P8OUT = BIT1; // turn off main regulator on the board
}

int main(void){

	WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer to prevent time out reset

	_delay_cycles(100000);

	P7SEL &= ~BIT6;
	P7DIR |= BIT6;
	P7OUT &= ~BIT6;

	#ifdef LIPOLY  //Turning on the board //Li-poly version
	P8SEL &= ~BIT1;
	P8DIR |= BIT1;
	P2SEL &= ~BIT6;
	P2DIR &= ~BIT6;
    P2REN |= BIT6;                          // Enable P2.6 internal resistance
    P2OUT &= ~BIT6;                          // Set P2.6 as pull-down resistance
	if((P2IN & BIT6)!=0)
		P8OUT &= ~BIT1;
	else
		P8OUT |= BIT1;

	P8OUT &= ~BIT3;

    P2IFG &= ~BIT6;                         // P2.6 IFG cleared
	P2IE &= ~BIT6;                           // P2.6 interrupt disabled

	while((P2IN & BIT6)!=0);

	P2REN |= BIT6;                          // Enable P2.6 internal resistance
	P2OUT &= ~BIT6;                          // Set P2.6 as pull-down resistance
    P2IE |= BIT6;                           // P2.6 interrupt enabled
    P2IES &= ~BIT6;                          // P2.6 low->high edge
    P2IFG &= ~BIT6;                         // P2.6 IFG cleared

	if ((AUXCTL0 & AUX0SW) != AUX0SW) // make sure VCC supply is ON
		turnOff();

	#else

	P2SEL &= ~BIT6;
	P2DIR &= ~BIT6;
    P2REN |= BIT6;                          // Enable P2.6 internal resistance
    P2OUT |= BIT6;                          // Set P2.6 as pull-up resistance

    if((P2IN & BIT6)==0)
    	lowbatt();

    P2IE |= BIT6;                           // P2.6 interrupt enabled
    P2IES |= BIT6;                          // P2.6 high->low edge
    P2IFG &= ~BIT6;                         // P2.6 IFG cleared

	#endif

    boardstatus = 125; //ON
	P7OUT &= ~BIT6;

	F_stop = DO_NOTHING; //need a stop function to set the initial mod
	__disable_interrupt();
	(*F_stop)();
	F_init = TURBINE_INIT;	// Just designate the actual functions of the function pointers... Since we are using turbine_mode,
	F_run = TURBINE_RUN;  // all F_init, F_ta3_cc would refer to turbine functions
	F_stop = TURBINE_STOP;
	F_ta3_cc = TURBINE_TA3_CC_INT;
	F_save = TURBINE_SAVE;
	__delay_cycles(100);
	(*F_init)();

	while(1) {(*F_run)();}	// check the status and write onto SD card forever
}

#pragma vector=TIMER3_A0_VECTOR // Timer A2 (capture/compare 0) Sampling data
__interrupt void TA3_CC0_ISR(void){
	(*F_ta3_cc)(); // Start reading data
	TA3CCTL0 &= ~CCIFG;
}

#pragma vector=PORT2_VECTOR // Port 2 interrupt service routine
__interrupt void Port_2(void){

	P2IFG &= ~BIT6; // P2.6 IFG cleared

	if(boardstatus==125){

		__disable_interrupt(); //Avoid jumping into ISR
		(*F_save)();
		turnOff();}

	else{

		P8SEL &= ~BIT1;
		P8DIR |= BIT1;
		P8OUT &= ~BIT1;
		PMMCTL0_H = PMMPW_H; // open PMM
		PMMCTL0_L |= PMMSWBOR;}
}
