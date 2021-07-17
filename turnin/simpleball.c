#include <avr/io.h>
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

enum Paddle1_States {Start1, Wait, Left, Right};

int Paddle1(int state) {
	unsigned char butA = ~PINA & 0x03;
//	unsigned char pad = 0x0F;
	static unsigned char pad;
	PORTC = 0x01; //stay in same column 

	switch(state) {
	case Start1:
	state = Wait;
	break;

	case Wait:
	if(butA == 0x02) { //press A1, left
	state = Left;
	}
	else if(butA == 0x01) { //press A0, right
	state = Right;
	}
	break;

	case Left:
	state = Wait;
	break;

	case Right:
	state = Wait;
	break;

	default:
	state = Start1;
	}//end of transitions

	switch(state) {
	case Start1:
	pad = 0xF0;
	PORTC = 0x01;
	break;

	case Right:
	if(pad == 0xF0) {
	pad = 0xF0;
	}
	else {
	pad = ~pad;
	pad = pad >> 1;
	pad = ~pad;
	}
	break;

	case Left:
	if(pad == 0x0F) {
	pad = 0x0F;
	}
	else {
	pad = ~pad;
	pad = pad << 1;
	pad = ~pad;
	}
	break;
	}//end of actions

	PORTD = pad;
	return state;
}//end of Paddle1



//static unsigned char ball_row;
//static unsigned char ball_col;
enum ball_states{Init, Forward, Back};
int ball(int state) {
	static unsigned char ball_row = 0xFB;
	static unsigned char ball_col = 0x01;
	
	switch(state) {
	case Init:
	state = Forward;
	break;

	case Forward:
	if(ball_col == 0x80) {
	state = Back;
	}
	break;

	case Back: 
	if(ball_col == 0x01) {
	state = Forward;
	}
	break;

	default:
	state = Init;
	break;
	}//end of transitions

	switch(state) {
	case Init:
	ball_col = 0x01;
	break;

	case Forward:
	if(ball_col == 0x80) {
	break;	
	}
	else {
	ball_col = ball_col << 1;
	}
	break;

	case Back:
	if(ball_col == 0x01) {
	break;
	}
	else {
	ball_col = ball_col >> 1;
	}
	break;

	}//end of actions

	PORTD = ball_row;
	PORTC = ball_col;

	return state;
}//end of ball

int main(){
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    task1.state = start;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &ball;

    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for (i = 1; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }


    TimerSet(GCD);
    TimerOn();

    while(1){
        for(i = 0; i < numTasks; i++){
            if(tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 0;
}
