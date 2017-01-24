// Sound.c

#include "DAC.h"
#include "Sound.h"
#include "..//tm4c123gh6pm.h"
#include <stdio.h>
#include <stdlib.h>
#include "Sound_Clips.h"
#define CAPACITY 16 

enum fsm_state 
	PLAYING, 
	IDLE, 
	LEVEL_OVER
};

typedef void (*SoundPlay)(void);
unsigned long sound_index = 0;
unsigned char level_end = 0;
const unsigned char *Wave;
unsigned long sclip_index = 0;
void (*sound_buffer[16])(void) ; 
unsigned char size_ = 0;
unsigned char tail_ = 0, head_ =0;	
unsigned char start = 0;
enum fsm_state curr_state = IDLE; 


//function that manipulates the sound buffer 
unsigned char enqueue(void (*func)(void))
{
	if(size_ >= CAPACITY) {
		return 0;
	} 
	else { 
		size_++;
		sound_buffer[tail_] = *func;
		tail_ = (tail_+1)%CAPACITY;
		return 1;
	}
}
//function that manipulates the sound buffer 
SoundPlay dequeue()
{
	if(size_ <= 0) {
		return NULL; 
	}
	else { 
		SoundPlay dequeuedItem = sound_buffer[head_]; 
		head_ = (head_+1)%CAPACITY; 
		size_--;
		return dequeuedItem;
	}
}


//functions to control the timer interrupts
void Timer2A_Stop(void){ 
  TIMER2_CTL_R &= ~0x00000001; // disable
}
void Timer2A_Start(void){ 
  TIMER2_CTL_R |= 0x00000001;   // enable
}


//initialization functions
void Sound_Init(void){
  DAC_Init();               // initialize simple 4-bit DAC
  Timer2_Init(80000000/11025);     // 11.025 kHz
  sound_index = 0;
  sclip_index = 0;
}

void Timer2_Init(unsigned long period){ 
  unsigned long volatile delay;
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  delay = SYSCTL_RCGCTIMER_R;
  TIMER2_CTL_R = 0x00000000;   // 1) disable timer2A
  TIMER2_CFG_R = 0x00000000;   // 2) 32-bit mode
  TIMER2_TAMR_R = 0x00000002;  // 3) periodic mode
  TIMER2_TAILR_R = period-1;   // 4) reload value
  TIMER2_TAPR_R = 0;           // 5) clock resolution
  TIMER2_ICR_R = 0x00000001;   // 6) clear timeout flag
  TIMER2_IMR_R = 0x00000001;   // 7) arm timeout
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) set to highest priority
  NVIC_EN0_R = 1<<23;          // 9) enable IRQ 23 in
  TIMER2_CTL_R = 0x00000001;   // 10) enable timer2A
}



//This ISR implments a simple FSM controller to manage output to the DAC based on game state
void Timer2A_Handler(void){ 
  TIMER2_ICR_R = 0x00000001;   // acknowledge timer2A Interrupt
  switch(curr_state)
  {
  	case IDLE:
  		//idle state with no sounds
  		if(level_end) {
  			curr_state = LEVEL_OVER;
  		}
		else if (size){ 
			curr_state = PLAYING; 
		}
		break;	
	case PLAYING:
		//dequeue from sound buffer to play new sound or continue playing current sound 
		if(sclip_index){
    		DAC_Out(Wave[sound_index]>>4);
    		++sound_index;
    		--sclip_index;
  		}
		else if(!sclip_index && size_) {
			(*dequeue())();         
  		}
  		if(level_end) { 
  			curr_state = LEVEL_OVER
  		}
  		if(!size) { 
  			curr_state = IDLE; 
  		}
  		break; 
  	case  LEVEL_END:
	  	// clear the sound buffer and reset all indexes 
  		size_ = 0; 
  		level_end = 0;
  		sclip_index = 0;
  		curr_state = IDLE;
  		break;
  }
}
//function that enables the main game loop to signal that a level is over
void Level_Stop(void) { 
	level_end = 1;
}

//set of functions that are enqueued to the sound buffer to play different sounds
// unavalible to the main game loop 
static void Sound_Shoot(void){
	Wave = shoot; 
	sound_index = 0;
    sclip_index = 4080;
}
static void Sound_Killed(void){
	Wave = invaderkilled; 
	sound_index = 0;
    sclip_index = 3377;
}
static void Sound_Explosion(void){
	Wave = explosion; 
	sound_index = 0;
    sclip_index = 2000;
}
static void Sound_Fastinvader1(void){
	Wave = fastinvader1; 
	sound_index = 0;
    sclip_index = 982;
}
static void Sound_Fastinvader2(void){
	Wave = fastinvader2; 
	sound_index = 0;
    sclip_index = 1041;
}
static void Sound_Highpitch(void){
	Wave = highpitch; 
	sound_index = 0;
    sclip_index = 1802;
}

//set of functions that the main game loop can call to play a sound clip 
unsigned char Play_Shoot(void){
	 SoundPlay func = Sound_Shoot;
	 return enqueue(func);
}
unsigned char Play_Killed(void){
	SoundPlay func = Sound_Killed;
  	return enqueue(func);
}
unsigned char Play_Explosion(void){
	SoundPlay func = Sound_Explosion;
  	return enqueue(func);
}
unsigned char Play_Fastinvader1(void){
	SoundPlay func = Sound_Fastinvader1;
  	return enqueue(func);
}
unsigned char Play_Fastinvader2(void){
	SoundPlay func = Sound_Fastinvader2;
  	return enqueue(func);
}
unsigned char Play_Highpitch(void){
	SoundPlay func = Sound_Highpitch;
  	return enqueue(func);
}

