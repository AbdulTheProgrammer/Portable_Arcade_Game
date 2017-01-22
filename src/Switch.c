#include "Switch.h"
#include "..//tm4c123gh6pm.h"


#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define PE1   (*((volatile unsigned long *)0x40024008)) 	
#define PE0   (*((volatile unsigned long *)0x40024004)) 	
#define PE3   (*((volatile unsigned long *)0x40024020)) 	
#define PE4   (*((volatile unsigned long *)0x40024040)) 
	
void Init_Switches() { 
	volatile unsigned long delay;	
	SYSCTL_RCGC2_R |= 0x00000010;     // Port E clock
  delay = SYSCTL_RCGC2_R;           // wait 3-5 bus cycles
  GPIO_PORTE_DIR_R &= ~0x01;        // PE0 input
  GPIO_PORTE_DIR_R &= ~0x02;        // PE1 input 
	GPIO_PORTE_DIR_R |= 0x18;        	// PE3, PE4 output
  GPIO_PORTE_AFSEL_R &= ~0x17;      // not alternative
  GPIO_PORTE_AMSEL_R &= ~0x17;      // no analog
  GPIO_PORTE_PCTL_R &= ~0x000FF0FF; // bits for PE1,PE0 PE3, PE4 GPIO
  GPIO_PORTE_DEN_R |= 0x1B;         // enable PE1,PE0, PE3,PE4 Digital IO 
}

unsigned long Read_Switch1() { 
	return PE1;
}
unsigned long Read_Switch0() { 
	return PE0;
}
void setPE3State (unsigned long state) { 
	PE3 = state;
}
void setPE4State (unsigned long state) { 
	PE4 = state;
}

void TogglePE3 (void) { 
	PE3 = PE3^0x08;
}

void TogglePE4 (void) { 
	PE4 = PE4^0x10; 
}
