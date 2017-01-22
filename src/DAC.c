// DAC.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Implementation of the 4-bit digital to analog converter
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// Port B bits 3-0 have the 4-bit DAC

#include "DAC.h"
#include "..//tm4c123gh6pm.h"

// **************DAC_Init*********************
// Initialize 4-bit DAC 
// Input: none
// Output: none
void DAC_Init(void){volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x02;          // 1) activate Port B
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // 2) no need to unlock PC7-0
  GPIO_PORTC_AMSEL_R = 0x00;       // 3) disable analog function on PC7-0
  GPIO_PORTC_PCTL_R = 0x00000000;  // 4) configure PC7-0 as GPIO
  GPIO_PORTC_DIR_R = 0x3F;         // 5) make PC7-0 out  
  GPIO_PORTC_AFSEL_R = 0x00;       // 6) disable alt funct on PC7-0
  GPIO_PORTC_DR8R_R = 0x3F;        // enable 8 mA drive on PC7-0
  GPIO_PORTC_DEN_R = 0x3F;         // 7) enable digital I/O on PB7-0
}


// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Output: none
void DAC_Out(unsigned long data){
  GPIO_PORTC_DATA_R = data;
}
