// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// November 19, 2012


void Sound_Init(void);
void Sound_Play(const unsigned char *pt, unsigned long count);
void Sound_Shoot(void);
void Sound_Killed(void);
void Sound_Explosion(void);

void Sound_Fastinvader1(void);
void Sound_Fastinvader2(void);
void Sound_Fastinvader3(void);
void Sound_Fastinvader4(void);
void Sound_Highpitch(void);
void clearSoundBuffer(void);
void Timer2_Init(unsigned long period);
unsigned char Play_Shoot(void);
unsigned char Play_Killed(void);
unsigned char Play_Explosion(void);
unsigned char Play_Fastinvader1(void);
unsigned char Play_Fastinvader2(void);
unsigned char Play_Highpitch(void);

