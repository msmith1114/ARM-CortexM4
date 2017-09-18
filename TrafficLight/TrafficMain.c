// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "tm4c123gh6pm.h"

// Initialize SysTick with busy wait running at bus clock.
#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))

#define LIGHT                   (*((volatile unsigned long *)0x400050FC)) //PORT B
#define SENSOR                  (*((volatile unsigned long *)0x4002401C)) //PORT E
#define WALK										(*((volatile unsigned long *)0x40025028)) //PORT F

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

void Delay(void){unsigned long volatile time;
  time = 80000; // 0.05sec
  while(time){
   time--;
  }
}

void SysTick_Init(void){
	// Initalise systick
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}

void SysTick_Wait10ms(){
	// Function to delay for 10ms
  NVIC_ST_RELOAD_R = 8000000 - 1;  // number of counts to wait (80Mhz PLL)
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
void SysTick_Wait(unsigned long delay){
	// Written by Billy.Ljm
	// Fucntion to delay for integer multiples of 10ms
	unsigned long i;
	for(i=0; i < delay; i++)
		SysTick_Wait10ms();
}

void PortInit(void) {
	volatile unsigned long delay;
	SYSCTL_RCGC2_R = 0x32;     // 1) activate clock for Port F and B and E
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
	GPIO_PORTB_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port D
  GPIO_PORTF_CR_R = 0xA;           // allow changes to PF1 and PF3
	GPIO_PORTB_CR_R = 0x3F;           // allow changes to PB5-0
	GPIO_PORTE_CR_R = 0x7;           // allow changes to PE2-0
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
	GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog on PD
	GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog on PE
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF
	GPIO_PORTE_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PE
	GPIO_PORTB_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PD
  GPIO_PORTF_DIR_R = 0xA;          // 5) PF3,PF1 OUT
	GPIO_PORTB_DIR_R = 0x3F;          // 5) PB5-PB0 OUT
	GPIO_PORTE_DIR_R &= ~0x7;          // 5) PE2-PE0 IN
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
	GPIO_PORTB_AFSEL_R = 0x00;        // 6) disable alt funct on PB7-0
	GPIO_PORTE_AFSEL_R = 0x00;        // 6) disable alt funct on PE7-0
  GPIO_PORTF_DEN_R = 0xA;          // 7) enable digital I/O on PF3 and PF1
	GPIO_PORTB_DEN_R = 0x3F;          // 7) enable digital I/O on PB5-0
	GPIO_PORTE_DEN_R = 0x7;          // 7) enable digital I/O on PE2-0
}

// ***** 3. Subroutines Section *****
// Linked data structure
struct State {
  unsigned long Out; //LED Output
	unsigned long WalkOut; //Walk LED Output
  unsigned long Time;  //Delay
  unsigned long Next[8];}; 
typedef const struct State STyp;
#define GoWest        0
#define WaitWest      1
#define GoSouth       2 
#define WaitSouth     3
#define WalkOn        4
#define WalkFlashOff1 5
#define WalkFlashOn1 6
#define WalkFlashOff2 7
#define WalkFlashOn2  8
STyp FSM[9]={
 {0xC,0x2,5,{GoWest,GoWest,WaitWest,WaitWest,WaitWest,WaitWest,WaitWest,WaitWest}}, 
 {0x14,0x2,5,{GoSouth,GoSouth,GoSouth,GoSouth,WalkOn,WalkOn,GoSouth,GoSouth}},
 {0x21,0x2,5,{GoSouth,WaitSouth,GoSouth,WaitSouth,WaitSouth,WaitSouth,WaitSouth,WaitSouth}},
 {0x22,0x2,5,{GoWest,GoWest,GoWest,GoWest,WalkOn,GoWest,WalkOn,WalkOn}},
 {0x24,0x8,5,{WalkOn,WalkFlashOff1,WalkFlashOff1,WalkFlashOff1,WalkFlashOff1,WalkFlashOff1,WalkFlashOff1,WalkFlashOff1}},
 {0x24,0x0,5,{WalkOn,WalkFlashOn1,WalkFlashOn1,WalkFlashOn1,WalkFlashOn1,WalkFlashOn1,WalkFlashOn1,WalkFlashOn1}},
 {0x24,0x2,5,{WalkOn,WalkFlashOff2,WalkFlashOff2,WalkFlashOff2,WalkFlashOff2,WalkFlashOff2,WalkFlashOff2,WalkFlashOff2}},
 {0x24,0x0,5,{WalkOn,WalkFlashOn2,WalkFlashOn2,WalkFlashOn2,WalkFlashOn2,WalkFlashOn2,WalkFlashOn2,WalkFlashOn2}},
 {0x24,0x2,5,{WalkOn,GoWest,GoSouth,GoWest,WalkOn,GoWest,GoSouth,GoWest}},
};
unsigned long S;  // index to the current state 
unsigned long Input; 	
	
	
int main(void){ 
	PortInit();
	SysTick_Init();
  EnableInterrupts();
	
  while(1){
		
   LIGHT = FSM[S].Out;  // set lights
	 WALK = FSM[S].WalkOut; // set walk led
   SysTick_Wait(FSM[S].Time);
   Input = SENSOR;     // read sensors
   S = FSM[S].Next[Input];  
	  
  }
}

