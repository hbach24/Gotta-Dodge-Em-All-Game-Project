#include <avr/io.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "../header/timer.h"
//WORKING DRAFT 
typedef struct _task {
        signed char state;
        unsigned long int period;
        unsigned long int elapsedTime;
        int(*TickFct)(int);
} task;

unsigned char play_C;
unsigned char play_D;

enum player_States {Start1, Wait, Left, Right};

int player(int state) {
	unsigned char butA = ~PINA & 0x03;
//	unsigned char pad = 0x0F;
	static unsigned char player;
	play_C = 0x80; //stay in same column 

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
	player = 0xF7;
	play_C = 0x01;
	break;

	case Right:
	if(player == 0xFE) {
	player = 0xFE;
	}
	else {
	player = ~player;
	player = player >> 1;
	player = ~player;
	}
	break;

	case Left:
	if(player == 0x7F) {
	player = 0x7F;
	}
	else {
	player = ~player;
	player = player << 1;
	player = ~player;
	}
	break;
	}//end of actions

	play_D = player;
	return state;
}//end of player

enum Obstacles_States {Begin, Shoot};

unsigned char obst_C; //PORTC
unsigned char obst_D; //PORTD

int Obstacles(int state) {
static unsigned char column; //PORTC
static unsigned char row = 0xF7; //PORTD
unsigned char random;

//srand(0);

	switch(state) {
	case Begin:
	state = Shoot;
	break;

	case Shoot:
	state = Shoot;
	break;

	default:
	state = Begin;
	}//end of transitions

	switch(state) {
	case Begin:
	column = 0x01;
	random = rand()%((254 - 0) + 1) + 0;
	row = random;
	break;

	case Shoot:
	if(column == 0x80) {
	column = 0x01;
	random = rand()%((254 - 0) + 1) + 0;
	row = random;
	break;
	}
	else {
	column = column << 1;
	}
	break;
	}//end of actions

	obst_D = row;
	obst_C = column;
	return state;
}//end of Shoot

unsigned char obst2_C;
unsigned char obst2_D;

enum Obstacles2_states{Begin2, Shoot2};
int Obstacles2(int state) {
static unsigned char column; //PORTC
static unsigned char row = 0xF7; //PORTD
unsigned char random;
//	srand(time(0));
        
	switch(state) {
        case Begin2:
        state = Shoot2;
        break;

        case Shoot:
        state = Shoot2;
        break;

        default:
        state = Begin2;
        }//end of transitions

        switch(state) {
        case Begin2:
        column = 0x01;
        random = rand()%((20) + 1) + 0;
        row = random;
        break;

        case Shoot2:
        if(column == 0x80) {
        column = 0x01;
        random = rand()%((21 - 0) + 1) + 0;
        row = random;
        break;
        }
        else {
        column = column << 1;
        }
        break;
        }//end of actions

        obst2_D = row;
        obst2_C = column;
        return state;
}//end of Obstacles2

bool lost = 0;
unsigned char lose_C = 0x00;
unsigned char lose_D = 0x00;

enum lose_States{Start_Lose, Check, Collide, Defeat};
int lose(int state) {

unsigned char check_Row_Equal = play_D | obst_D;
//bit mask: if 0xFF, no collision, ELSE if other hex value, collision detected

        switch(state) {
        case Start_Lose:
        state = Check;
        break;

        case Check:
        if((check_Row_Equal != 0xFF) && (play_C == obst_C)) {
        state = Collide;
        }
        else {
        state = Check;
        }
        break;

        case Collide:
        state = Defeat;
        break;

        default:
        state = Start_Lose;
        break;
        }//end of transitions

        switch(state) {
        case Start_Lose:
        break;

        case Collide:
        lost = 1;
        break;

        case Defeat:
        lose_C = 0xFF;
        lose_D = 0x00;
        break;
        }
return state;
}//end of lose()

enum display_States{Start_Disp, Disp};
unsigned char output = 0x00;
int ind = 0;

int display(int state) {

unsigned char ROW[3] = {obst_D, obst2_D, play_D};
unsigned char COLUMN[3] = {obst_C, obst2_C, play_C};

	/*if(lost == 1) {
	PORTC = lose_C;
	PORTD = lose_D;
	}*/
	//else {
	switch(state) {
	case Start_Disp:
	state = Disp;
//	ind++;
	break;

	case Disp:
	state = Disp;
//	ind++;
	break;

	default:
	state = Start_Disp;
	break;
	}//end of transitions

	switch(state) {
	case Start_Disp:
	ind = 0;
	break;

	case Disp:
	if(ind >= 2) {
	PORTC = COLUMN[ind];
	PORTD = ROW[ind];
	ind = 0; //reset
	}
	else {
	PORTC = COLUMN[ind];
	PORTD = ROW[ind];
	ind++;
	}
	break;
	}//end of actions
	
	//PORTC = COLUMN[ind];
	//PORTD = ROW[ind];
return state;
	//}//end of else
	return 0;
}//end of display


int main(){
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    //srand(time(0));
    static task task1, task2, task3, task4, task5;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    task1.state = start;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &player;

    task2.state = start;
    task2.period = 200;
    task2.elapsedTime = task2.period;
    task2.TickFct = &Obstacles;

    task3.state = start;
    task3.period = 1;
    task3.elapsedTime = task3.period;
    task3.TickFct = &display;

    task4.state = start;
    task4.period = 210;
    task4.elapsedTime = task4.period;
    task4.TickFct = &Obstacles2;

    task5.state = start;
    task5.period = 1;
    task5.elapsedTime = task5.period;
    task5.TickFct = &display;

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
   	//PORTC = tmpC;
	//PORTD = tmpD;
   	}
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 0;
}
