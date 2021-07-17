#include <avr/io.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "../header/timer.h"

typedef struct _task {
        signed char state;
        unsigned long int period;
        unsigned long int elapsedTime;
        int(*TickFct)(int);
} task;

enum Shoot_States {Begin, Shoot};

int Obstacles(int state) {
static unsigned char column; //PORTC
static unsigned char row = 0xFE; //PORTD
unsigned char random;

	switch(state) {
	case Begin:
	state = Shoot;
	break;

	case Shoot:
	state = Shoot;
	break;
	}//end of transitions

	switch(state) {
	case Begin:
	column = 0x01;
	random = rand()%((255 - 0) + 1) + 0;
//	row = 0xFE;
	break;

	case Shoot:
	if(column == 0x80) {
	column = 0x01;
	random = rand()%((255 - 0) + 1) + 0;
//	row = 0xFE;
	break;
	}
	else {
	column = column << 1;
	}
	break;
	}//end of actions

	PORTD = row;
	PORTC = column;
	return state;
}//end of Shoot


int main(){
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    srand(time(NULL));
    const char start = -1;

    task1.state = start;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Obstacles;

    TimerSet(100);
    TimerOn();

    unsigned short i;
    while(1){
        for(i = 0; i < numTasks; i++){
            if(tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += 100;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 0;
}
