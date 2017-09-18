// ***** 0. Documentation Section *****
/**
 * UTAustinX: UT.6.10x Embedded Systems - Shape the World
 * Lab 10: Traffic Light
 *
 * File Name: BasicSwitch.c
 *
 * Description:
 * Simulates handling basic Input via Switches and Output Via LED
 *   
 * Author: Matt Smith
 * Created: August 1, 2017
 * 
 */

// ***** 1. Pre-processor Directives Section *****
#include "tm4c123gh6pm.h"
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define PE0_input								(*((volatile unsigned long *)0x40024004))
#define PE1_output 							(*((volatile unsigned long *)0x40024008))

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortE_Init(void);
void Delay1ms(unsigned long msec);

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor

int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  /*
	1) Make PE1 an output and make PE0 an input. 
	2) The system starts with the LED on (make PE1 =1). 
	3) Wait about 100 ms
	4) If the switch is pressed (PE0 is 1), then toggle the LED once, else turn the LED on. 
	5) Steps 3 and 4 are repeated over and over.
	*/
	PortE_Init();
  EnableInterrupts();           // enable interrupts for the grader
	PE1_output |= 0x2;
	Delay1ms(100);
  while(1){
		if(PE0_input == 0x1){
			//if switch pressed
			PE1_output ^= 0x2;
			Delay1ms(100);
		}
		else {
			PE1_output |= 0x2;
		}
		
    
  }
  
}

void PortE_Init(void){ 
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x10;           // Port E clock
  delay = SYSCTL_RCGC2_R;            // delay to allow clock to stabilize     
  GPIO_PORTE_AMSEL_R &= 0x00;        // 2) disable analog function
  GPIO_PORTE_PCTL_R &= 0x00000000;   // 3) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R &= ~0x1;         // 4.1) PE0 input,
  GPIO_PORTE_DIR_R |= 0x02;          // 4.2) PE1 output  
  GPIO_PORTE_AFSEL_R &= 0x00;        // 5) no alternate function
  GPIO_PORTE_PUR_R &= ~0x1;          // 6) disable pullup resistors      
  GPIO_PORTE_DEN_R |= 0x3;          // 7) enable digital pins PE0-PE1
}

/*
Description: Uses time of a instruction to delay time
Almost 1ms for each cycle
Input: Number of ms to delay
Output: None
*/
void Delay1ms(unsigned long msec){
// write this function
    unsigned long i;
   
  while(msec > 0){
    i = 13333;  // this number means 1ms
    while(i > 0){
      i = i - 1;
    }
    msec = msec - 1; // decrements every 1 ms
  }
}
