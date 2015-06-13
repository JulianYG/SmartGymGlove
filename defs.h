/* C Pre-Compiler Constant Definitions File */
#ifndef __DEFS
#define __DEFS

#include "msp430.h"

//mode to cause MSP430 to exit loop
#define SHUTDOWN_MODE 0x3F

//CLOCKS - GENERAL
#define ACLK 0
#define SMCLK 1
#define MCLK 2

//CLOCKS - USCI
#define UCACLK 3

//CLOCKS - UNIFIED CLOCK SYSTEM SETUP
#define MCLK_1MHZ 31
#define MCLK_2MHZ 63
#define MCLK_4MHZ 127
#define MCLK_8MHZ 255
#define MCLK_16MHZ 511
#define MCLK_24MHZ 732

//USCI TIMER TRIGGER FREQUENCY (USCI_<clock freq>_<frequency in Hz>)
#define USCI_32K_100 328
#define USCI_32K_252 130
#define USCI_32K_762 43
#define USCI_32K_993 33
#define USCI_4M_500 8388
#define USCI_4M_1000 4194
#define USCI_8M_250 33554
#define USCI_8M_500 16777
#define USCI_8M_750 11185

//USCI DEVICES
#define BT2 0

#endif
