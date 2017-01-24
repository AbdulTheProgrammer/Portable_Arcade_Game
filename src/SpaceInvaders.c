/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"
#include "Switch.h"
#include "DAC.h" 
#include "ADC.h" 
#include "Sound.h"
#include "SpaceInvaders.h"
#include "Game_Images.h"
#include <assert.h>

#define BUNKERW     ((unsigned char)Bunker0[18])
#define BUNKERH     ((unsigned char)Bunker0[22])
#define ENEMY30W    ((unsigned char)SmallEnemy30PointA[18])
#define ENEMY30H    ((unsigned char)SmallEnemy30PointA[22])
#define ENEMY20W    ((unsigned char)SmallEnemy20PointA[18])
#define ENEMY20H    ((unsigned char)SmallEnemy20PointA[22])
#define ENEMY10W    ((unsigned char)SmallEnemy10PointA[18])
#define ENEMY10H    ((unsigned char)SmallEnemy10PointA[22])
#define ENEMYBONUSW ((unsigned char)SmallEnemyBonus0[18])
#define ENEMYBONUSH ((unsigned char)SmallEnemyBonus0[22])
#define LASERW      ((unsigned char)Laser0[18])
#define LASERH      ((unsigned char)Laser0[22])
#define MISSILEW    ((unsigned char)Missile0[18])
#define MISSILEH    ((unsigned char)Missile0[22])
#define PLAYERW     ((unsigned char)PlayerShip0[18])
#define PLAYERH     ((unsigned char)PlayerShip0[22])
#define ENEMYASCENT 1
#define RESPAWNRATE 200 
#define EXPLOSIONLIFE 45 
#define RECHARGERATE 20 
#define MISSILESPEED 1 
#define LASERSPEED 1
#define SCREENW 84 
#define SCREENH	48

// game objects defined as structs

typedef struct State Styp {
  	unsigned long x;      // x coordinate
  	unsigned long y;      // y coordinate
  	const unsigned char *image[4]; // ptr->image
  	signed char life; 
	unsigned long points;
};          

//stuct for temporary game objects
typedef struct State1 Temp_Obj { 
	unsigned long x;      // x coordinate
  	unsigned long y;      // y coordinate
  	const unsigned char *image[2]; // ptr->image     
	unsigned long lifetime;
};

typedef struct PlayerState Player { 
	unsigned long x; 
	unsigned long y; 
	const unsigned char *image; 
	unsigned long lives; 
};

STyp Bunker; 
STyp Enemy[3][4];
STyp MotherShip;
Temp_Obj Explosions[32]; 
Temp_Obj LaserBuffer[32];
Temp_Obj MissileBuffer[32];
Player Player1; 

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

//control flag used for synchronization between the ISR and the main game loop
unsigned char Control_Flag;

unsigned char prev_btn_state = 0;

//variables used to index the appropriate buffers
unsigned char laser_index =0, misl_index = 0, exp_index =0;	

//enemy parameters which change at start of every new level
signed char EnemySpeed = 1; 
unsigned char EnemyRowCount = 2, enemySoundCounter =0, EnemyCount = 0;
unsigned char EnemyFireCounter =0, EnemyMoveCounter =0, EnemyMissileCounter =0;
unsigned long EnemyFireCounterMask = 20, EnemyMoveCounterMask = 18, EnemyMissileCounterMask = 5, EnemyFormCounter =0;
unsigned long periodicEnemyRate = RESPAWNRATE, LaserCounter = RECHARGERATE;

unsigned long SCORE = 0;

void Systick_Init(void){
  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = 2666666;     // reload value for 440hz (assuming 80MHz)
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0                
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts  
  DAC_Init();
}
//initializes all game parameters
void Init_Game(void){ char i =0, j =0, enemyType =0;
	EnemyCount= EnemyRowCount*4;
	Nokia5110_ClearBuffer();
	for(i=0;i<32;i++){ 
		LaserBuffer[i].x =0; 
		LaserBuffer[i].y =SCREENH-LASERH; 
		LaserBuffer[i].image[0] = Laser0; 
		LaserBuffer[i].image[1] = Laser1; 
		LaserBuffer[i].lifetime = 0;
		MissileBuffer[i].x =0; 
		MissileBuffer[i].y =0; 
		MissileBuffer[i].image[0] = Missile0; 
		MissileBuffer[i].image[1] = Missile1; 
		MissileBuffer[i].lifetime = 0;
		Explosions[i].x =0; 
		Explosions[i].y =0; 
		Explosions[i].image[0] = SmallExplosion0; 
		Explosions[i].image[1] = Points10; 
		Explosions[i].lifetime = 0;
	}
	for(i =0;i<EnemyRowCount;i++) {
		for(j=0;j<4;j++){
			Enemy[i][j].x = ENEMY10W*j;
			Enemy[i][j].y = ENEMY10H*(i+2);
			Enemy[i][j].life = 1;
			enemyType = ((Random32()>>24)%4)+1;
			if(enemyType==1) { 
				Enemy[i][j].image[0] = SmallEnemy10PointA;
				Enemy[i][j].image[1] = SmallEnemy10PointB;
				Enemy[i][j].points = 10;
			}
			else if (enemyType ==2) {
				Enemy[i][j].image[0] = SmallEnemy20PointA;
				Enemy[i][j].image[1] = SmallEnemy20PointB;
				Enemy[i][j].points = 20;
			}
			else {
				Enemy[i][j].image[0] = SmallEnemy30PointA;
				Enemy[i][j].image[1] = SmallEnemy30PointB;
				Enemy[i][j].points = 30;
			}
			Nokia5110_PrintBMP(Enemy[i][j].x, Enemy[i][j].y, Enemy[i][j].image[0], 0);
		}
	}
		MotherShip.y = ENEMYBONUSH; 
		MotherShip.x =0; 
		MotherShip.life = 1; 
		MotherShip.image[0] = SmallEnemyBonus0;
		MotherShip.image[1] = SmallEnemyBonus0; 
		MotherShip.points = 50;
		Player1.x = 0;
    Player1.y = SCREENH-1;
    Player1.image = PlayerShip0;
    Player1.lives = 3;
		Bunker.x = SCREENW/2 - BUNKERW/2;
		Bunker.y	=SCREENH-PLAYERH-1; 
		Bunker.life =3;
		Bunker.image[3] = Bunker0;
		Bunker.image[2] = Bunker1;
		Bunker.image[1] = Bunker2;
		Bunker.image[0] = Bunker3;
		Nokia5110_PrintBMP(Bunker.x, Bunker.y, Bunker.image[Bunker.life], 0);
		Nokia5110_PrintBMP(Player1.x, Player1.y, Player1.image, 0);
		Nokia5110_DisplayBuffer();
	 //Delay100ms(10000);
}

//signals for new laser object to be drawn on the screen
void LaserLaunch(unsigned long start) {
	setPE3State(0x08);
	laser_index = (laser_index +1)&0x0F;	
	LaserBuffer[laser_index].x = start; 
	LaserBuffer[laser_index].y = SCREENH-PLAYERH;
	LaserBuffer[laser_index].lifetime = 1;
	Play_Shoot();
	LaserCounter = RECHARGERATE; 
	//Sound_Shoot(); 
}

//signals for a new Motership object to be drawn on the screen
void MotherShipLaunch() { 
	setPE4State(0x10);
	Play_Highpitch();
	MotherShip.life = 1;
}

//signals for a new player missle to be drawn on the screen
void MissileLaunch() { 
	unsigned char EnemySelect;
	misl_index = (misl_index +1)&0x0F;
	if(EnemyRowCount==1) { 
		EnemySelect = ((Random32()>>24)%4);
	}
	else if (EnemyRowCount==2) { 
		EnemySelect = ((Random32()>>24)%8);
	}
	else {
			EnemySelect = ((Random32()>>24)%12);
	}
	if(Enemy[EnemySelect/4][EnemySelect%4].life) {
		MissileBuffer[misl_index].x =  Enemy[EnemySelect/4][EnemySelect%4].x+ENEMY10W/2;
		MissileBuffer[misl_index].y = Enemy[EnemySelect/4][EnemySelect%4].y+MISSILEH;
		MissileBuffer[misl_index].lifetime = 1500;
	}
}
//updates new enemy position and signals for new sounds to be played
void EnemyMove(void){ unsigned char i,j;
  unsigned char leftMost = 3;
	unsigned char rightMost = 0;
	unsigned char leftMostRow =0; 
	unsigned char rightMostRow =0;
	unsigned char alive = 0;
	for(i=0;i<EnemyRowCount;i++) { 
		for(j=0;j<4;j++) { 
			if(Enemy[i][j].life && j> rightMost) {
				rightMost =j;
				rightMostRow =i;
				alive =1;
			}
			if(Enemy[i][j].life && j< leftMost) { 
				leftMost =j;
				leftMostRow =i; 
				alive =1;
			}
		}
	}
	if(alive && (Enemy[rightMostRow][rightMost].x >= 68 || (Enemy[leftMostRow][leftMost].x <= 0 && EnemySpeed < 0))){
				EnemySpeed *= -1;
		
		if(Bunker.life <= 0) {
			for(i=0;i<EnemyRowCount;i++) { 
				for(j=0;j<4;j++){
						Enemy[i][j].y += ENEMYASCENT;
					}
				}
			}
		}
	for(i=0;i<EnemyRowCount;i++) { 
		for(j=0;j<4;j++){
			Enemy[i][j].x += EnemySpeed;
		}
	}
	EnemyFormCounter = (EnemyFormCounter+1)&0x01;
	enemySoundCounter = (enemySoundCounter+1)&0x01;
		if(!enemySoundCounter) {
			Play_Fastinvader1();
		}
		else if (enemySoundCounter == 1){
			Play_Fastinvader2();
		}
}
//function responsible to redrawing all the game objects and checking for collisions between enemy/players and projectiles, called from main game loop
void Draw(void){ unsigned char i,j,k;
	Nokia5110_ClearBuffer();
	for(i=0;i<32;i++) {
		if(LaserBuffer[i].lifetime) { 
			if(LaserBuffer[i].x >= Bunker.x && LaserBuffer[i].x+LASERW <= Bunker.x+BUNKERW && LaserBuffer[i].y-LASERH <= Bunker.y && Bunker.life) { 
				Bunker.life--;
				LaserBuffer[i].lifetime =0;
				if(!Bunker.life) { 
						Explosions[exp_index].x = Bunker.x; 
						Explosions[exp_index].y = Bunker.y;
						Explosions[exp_index].image[0] = BigExplosion0; 
						Explosions[exp_index].image[1] = BigExplosion0;
						Explosions[exp_index].lifetime = ExplosionLife;
						exp_index = (exp_index+1)&0x1F;
						Play_Explosion(); 
				}
			}
			if(MotherShip.life && LaserBuffer[i].x >= MotherShip.x && LaserBuffer[i].x+LASERW <= MotherShip.x+ENEMYBONUSW && LaserBuffer[i].y-LASERH <= MotherShip.y) {
					MotherShip.life = 0;
					MotherShip.x =0;
					Explosions[exp_index].x = MotherShip.x; 
					Explosions[exp_index].y = MotherShip.y;
					Explosions[exp_index].image[0] = SmallExplosion0; 
					Explosions[exp_index].image[1] = Points50;
					Explosions[exp_index].lifetime = ExplosionLife;
					exp_index = (exp_index+1)&0x1F;
					Play_Killed(); 
					SCORE += MotherShip.points;
			}
			for(k=0;k<EnemyRowCount;k++) {
				for(j=0;j<4;j++) {
					if(LaserBuffer[i].x >= Enemy[k][j].x && LaserBuffer[i].x+LASERW <= Enemy[k][j].x+ENEMY10W && LaserBuffer[i].y-LASERH <= Enemy[k][j].y && Enemy[k][j].life) { 
						Enemy[k][j].life--;
						LaserBuffer[i].lifetime =0; 
						Explosions[exp_index].x = Enemy[k][j].x; 
						Explosions[exp_index].y = Enemy[k][j].y;
						Explosions[exp_index].image[0] = SmallExplosion0; 
						Play_Killed();
						EnemyCount--; 
						SCORE += Enemy[k][j].points;
						if(Enemy[k][j].points == 10) {
							Explosions[exp_index].image[1] = Points10; 
						}
						else if(Enemy[k][j].points == 20) {
							Explosions[exp_index].image[1] = Points20; 
						}
						else {
							Explosions[exp_index].image[1] = Points30; 
						}
						Explosions[exp_index].lifetime = ExplosionLife;
						exp_index = (exp_index+1)&0x1F;
					}
				}
			}
		}
		
		if(MissileBuffer[i].lifetime) { 
			if(MissileBuffer[i].x >= Player1.x && MissileBuffer[i].x+MISSILEW <= Player1.x+PLAYERW && MissileBuffer[i].y >= Player1.y-PLAYERH) { 
				MissileBuffer[i].lifetime =0; 
				Player1.lives--;
			}
			if(MissileBuffer[i].x >= Bunker.x && MissileBuffer[i].x+MISSILEW <= Bunker.x+BUNKERW && MissileBuffer[i].y >= Bunker.y-BUNKERH && Bunker.life) { 
				Bunker.life--;
				MissileBuffer[i].lifetime =0; 
				if(!Bunker.life) { 
						Explosions[exp_index].x = Bunker.x; 
						Explosions[exp_index].y = Bunker.y;
						Explosions[exp_index].image[0] = BigExplosion0; 
						Explosions[exp_index].image[1] = BigExplosion1;
						Explosions[exp_index].lifetime = ExplosionLife;
						exp_index = (exp_index+1)&0x1F;
						Play_Explosion(); 
				}
			}
			if(MissileBuffer[i].y >= SCREENH-1) {
				MissileBuffer[i].lifetime =0;
			}
			for(k=0;k<32;k++) {
				if(MissileBuffer[i].lifetime && LaserBuffer[k].lifetime && MissileBuffer[i].x >= LaserBuffer[k].x && LaserBuffer[k].x+LASERW <= MissileBuffer[i].x+MISSILEW && LaserBuffer[k].y-LASERH <= MissileBuffer[i].y) { 
						MissileBuffer[i].lifetime=0;
						LaserBuffer[k].lifetime =0; 
						Explosions[exp_index].x = LaserBuffer[k].x; 
						Explosions[exp_index].y = LaserBuffer[k].y;
						Explosions[exp_index].image[0] = SmallExplosion0; 
						Explosions[exp_index].image[1] = Points10;
						Explosions[exp_index].lifetime = ExplosionLife;
						exp_index = (exp_index+1)&0x1F;
						SCORE += 10;
						Play_Explosion();  
				}
			}
		}
		if(LaserBuffer[i].y <= 0) {
				LaserBuffer[i].lifetime =0;
			}
		if(LaserBuffer[i].lifetime) { 
			LaserBuffer[i].y -= LASERSPEED;
			Nokia5110_PrintBMP(LaserBuffer[i].x, LaserBuffer[i].y,LaserBuffer[i].image[0],0);
		}
		if(MissileBuffer[i].lifetime){ 
			if(!EnemyMissileCounter) { 
			MissileBuffer[i].y += MISSILESPEED;
			}
			Nokia5110_PrintBMP(MissileBuffer[i].x, MissileBuffer[i].y,MissileBuffer[i].image[0],0);
		}
	}
	for(i=0;i<EnemyRowCount;i++) { 
		for(j=0;j<4;j++){
			if(Enemy[i][j].life){
			 Nokia5110_PrintBMP(Enemy[i][j].x, Enemy[i][j].y, Enemy[i][j].image[EnemyFormCounter], 0);
			}
		}
	}
	//draws hearts corresponding to player lives
	if(!Player1.lives) { 
		Nokia5110_PrintBMP(SCREENW-16, 10, empty_heart, 0);
		Nokia5110_PrintBMP(SCREENW-32, 10, empty_heart, 0);
		Nokia5110_PrintBMP(SCREENW-48, 10, empty_heart, 0);
	}
	if(Player1.lives == 1) { 
		Nokia5110_PrintBMP(SCREENW-16, 10, Heart, 0);
		Nokia5110_PrintBMP(SCREENW-32, 10, empty_heart, 0);
		Nokia5110_PrintBMP(SCREENW-48, 10, empty_heart, 0);
	}
	if(Player1.lives == 2) { 
		Nokia5110_PrintBMP(SCREENW-16, 10, Heart, 0);
		Nokia5110_PrintBMP(SCREENW-32, 10, Heart, 0);
		Nokia5110_PrintBMP(SCREENW-48, 10, empty_heart, 0);
	}
	if(Player1.lives == 3) { 
		Nokia5110_PrintBMP(SCREENW-16, 10, Heart, 0);
		Nokia5110_PrintBMP(SCREENW-32, 10, Heart, 0);
		Nokia5110_PrintBMP(SCREENW-48, 10, Heart, 0);
	}
	if(Bunker.life) { 
		Nokia5110_PrintBMP(Bunker.x, Bunker.y, Bunker.image[Bunker.life], 0);
	}
	if(MotherShip.life) {  
		Nokia5110_PrintBMP(MotherShip.x, MotherShip.y, MotherShip.image[0], 0);
		MotherShip.x++;
		TogglePE4();
	}
	if(MotherShip.x> (SCREENW-ENEMYBONUSW)) { 
		MotherShip.life = 0;
		MotherShip.x =0;
		setPE4State(0x00);
	}
	Nokia5110_PrintBMP(Player1.x, Player1.y, Player1.image, 0);
	for(i=0;i<32;i++){ 
			if(Explosions[i].lifetime) { 
				if(Explosions[i].lifetime <= ExplosionLife/2 && Explosions[i].image[0] == SmallExplosion0) {
					Nokia5110_PrintBMP(Explosions[i].x, Explosions[i].y,Explosions[i].image[1],0); 
				}
				else { 
					Nokia5110_PrintBMP(Explosions[i].x, Explosions[i].y,Explosions[i].image[0],0); 
				}
				Explosions[i].lifetime--;
		}
	}
  Nokia5110_DisplayBuffer(); 
	if(!MotherShip.life || MotherShip.x > SCREENW/2) {
		Nokia5110_OutUDec(SCORE);	
	}
	if(!Player1.lives) { 
		Delay100ms(20);
	}		
	Nokia5110_SetCursor(1, 1);

}

unsigned char Start_Game() { 
while(Player1.lives && EnemyCount){
		while(!Control_Flag){}			
		Draw();
		Control_Flag = 0;
  }
	if(EnemyCount) { 
		return 0;
	}
	else {
		return 1;
	}
}

void Game_Over() { 
		Stop_Sound();
		DisableInterrupts(); 
		Nokia5110_Clear();
		Nokia5110_ClearBuffer();
		Nokia5110_SetCursor(1, 1);
		Nokia5110_OutString("GAME OVER");
		Nokia5110_SetCursor(1, 2);
		Nokia5110_OutString("SCORE:");
		Nokia5110_SetCursor(1, 3);
		Nokia5110_OutUDec(SCORE);	
		Nokia5110_SetCursor(1, 4);
		Nokia5110_OutString("PRESS PE0");	
		while(!Read_Switch0());
		SCORE =0;
}

//main game state control loop 
int main(void){
	unsigned char prev_btn_state =0;
  TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
  Nokia5110_Init(); 
	ADC0_Init();
	Init_Switches();
	Sound_Init();
	Nokia5110_Clear();
	while(1) {unsigned char alive;
		setPE3State(0x00); 
		setPE4State(0x00);
		EnableInterrupts();
		EnemyRowCount= 1;
		EnemyFireCounterMask = 18;
		EnemyMoveCounterMask = 18; 
		EnemyMissileCounterMask = 10;
		Nokia5110_ClearBuffer();
		Nokia5110_SetCursor(1, 1);
		Nokia5110_OutString("PRESS PE0");
		Nokia5110_SetCursor(1, 2);
		Nokia5110_OutString("TO START");
		Nokia5110_SetCursor(1, 3);
		Nokia5110_OutString("THE GAME");	
		Nokia5110_SetCursor(1, 4);
		Nokia5110_OutString("EARTHLING!");	
		
		while(!Read_Switch0()) {
			Play_Highpitch();
		}
		Stop_Sound();
		Random_Init(NVIC_ST_CURRENT_R);
		Systick_Init();
		Init_Game();
		alive = Start_Game(); 
		// level 1 complete
		if(alive) { 
			setPE3State(0x00); 
			setPE4State(0x00);
			Player1.lives = 3;
			Stop_Sound();
			DisableInterrupts(); 
			Nokia5110_Clear();
			Nokia5110_ClearBuffer();
			Nokia5110_SetCursor(1, 1);
			Nokia5110_OutString("~LEVEL 2~");
			Nokia5110_SetCursor(1, 2);
			Nokia5110_OutString("SCORE:");
			Nokia5110_SetCursor(1, 3);
			Nokia5110_OutUDec(SCORE);	
			Nokia5110_SetCursor(1, 4);
			Nokia5110_OutString("PRESS PE0");	
			while(!Read_Switch0());
			EnableInterrupts(); 
			EnemyRowCount= 2;
			EnemyFireCounterMask = 14;
			EnemyMoveCounterMask = 14; 
			EnemyMissileCounterMask = 8;
			Init_Game();
			alive = Start_Game();
			// level 2 complete 
			if(alive) { 
				setPE3State(0x00); 
				setPE4State(0x00);
				Player1.lives = 3;
				Stop_Sound();
				DisableInterrupts(); 
				Nokia5110_Clear();
				Nokia5110_ClearBuffer();
				Nokia5110_SetCursor(1, 1);
				Nokia5110_OutString("~LEVEL 3~");
				Nokia5110_SetCursor(1, 2);
				Nokia5110_OutString("SCORE:");
				Nokia5110_SetCursor(1, 3);
				Nokia5110_OutUDec(SCORE);	
				Nokia5110_SetCursor(1, 4);
				Nokia5110_OutString("PRESS PE0");	
				while(!Read_Switch0());
					EnemyRowCount= 2;
					EnemyFireCounterMask = 18;
					EnemyMoveCounterMask = 5; 
					EnemyMissileCounterMask = 5;
					EnableInterrupts(); 
					Init_Game();
					alive = Start_Game();
					//level 3 complete 
					if(alive) { 
						Stop_Sound();
						DisableInterrupts(); 
						Nokia5110_Clear();
						Nokia5110_ClearBuffer();
						Nokia5110_SetCursor(1, 1);
						Nokia5110_OutString("YOU WIN");
						Nokia5110_SetCursor(1, 2);
						Nokia5110_OutString("SCORE:");
						Nokia5110_SetCursor(1, 3);
						Nokia5110_OutUDec(SCORE);	
						Nokia5110_SetCursor(1, 4);
						Nokia5110_OutString("PRESS PE0");	
						while(!Read_Switch0());
						EnableInterrupts(); 					
					}
					else { 
						setPE3State(0x00); 
						setPE4State(0x00);
						Game_Over();
					}
			}
			else { 
					setPE3State(0x00); 
					setPE4State(0x00);
				Game_Over();
			}
		}
		else {
					setPE3State(0x00); 
					setPE4State(0x00);		
			Game_Over();
		}
	}

}
//ISR used to update the various state changes within the game with counters
void SysTick_Handler(void){
	EnemyFireCounter = (EnemyFireCounter+1)%EnemyFireCounterMask;
	EnemyMoveCounter = (EnemyMoveCounter+1)%EnemyMoveCounterMask; 
	EnemyMissileCounter = (EnemyMissileCounter+1)%EnemyMissileCounterMask;
	if(!EnemyMoveCounter) {
		EnemyMove();
	}
	periodicEnemyRate--;
	if(!periodicEnemyRate) { 
		MotherShipLaunch();
		periodicEnemyRate = RESPAWNRATE;
	}	
	if(!EnemyFireCounter) { 
		MissileLaunch();
	}
	if(prev_btn_state && !Read_Switch1() && !LaserCounter) { 
		LaserLaunch(Player1.x+(PLAYERW/2)); 
	}
	if(LaserCounter) { 
		LaserCounter--;
	}
	else { 
		setPE3State(0x00);
	}
	prev_btn_state = Read_Switch1();
	//updates the position of the player according to what is read in by the ADC/potentionmeter
	Player1.x = ADC0_In()*0.016122; /* calibrated value */
	Control_Flag =1;
}
//generic delay function
void Delay100ms(unsigned long count){unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
