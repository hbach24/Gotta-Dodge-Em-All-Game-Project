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

int main(){
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    task1.state = start;
    task1.period = 50;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Paddle1;

    TimerSet(50);
    TimerOn();

    unsigned short i;
    while(1){
        for(i = 0; i < numTasks; i++){
            if(tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += 50;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 0;
}
