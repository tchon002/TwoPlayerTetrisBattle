#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "C:\Users\student\Documents\Atmel Studio\include\timer.h"
#include "C:\Users\student\Documents\Atmel Studio\include\io.c"
#include "C:\Users\student\Documents\Atmel Studio\include\usart.h"
#include <math.h>
#include <string.h>

//--------Find GCD function -------------------------------
unsigned long int findGCD (unsigned long int a, unsigned long int b)
{
    unsigned long int c;
    while(1){
        c = a%b;
        if(c==0){return b;}
        a = b;
        b = c;
    }
    return 0;
}


//ARI HELPEDD
void sendArduino(unsigned char val){
	while(!USART_IsSendReady(0));
	USART_Send(val, 0);
	while(!USART_HasTransmitted(0));
	asm("nop");
}

typedef struct _task {
    signed char state;
    unsigned long int period;
    unsigned long int elapsedTime;
    int (*TickFct)(int);
} task;

//LCD Display Tetris
unsigned char x = 0;
unsigned char pause = 0;

char message[] = "                Welcome To Tetris! Press Button To Start.                 ";
unsigned char front = 0;
unsigned char back = 15;
char substr[17];
unsigned char j = 0;

enum LCD_States{LCD_start} LCD_state;
int LCD_Tick()
{
    switch(LCD_state)
    {
        case LCD_start:
            if(x >= 25){
                if(back >= 74)
                {
                    back = 15;
                    front = 0;
                    j = 0;
                }
                strncpy(substr, message + front, 16);
                substr[16] = '\0';
                if(front < 16)
                {
                    j++;
                }
                LCD_DisplayString(1,substr);
                LCD_Cursor(1);
                front++;
                back++;
                x = 0;
            }
            x++;
            return LCD_start;
    }
    return LCD_start;
}
//End of LCD Display

//Beginning of PWM Song //Task2
void set_PWM(double frequency) {
    static double current_frequency;
    if (frequency != current_frequency) {
        
        if (!frequency) TCCR3B &= 0x08; //stops timer/counter
        else TCCR3B |= 0x03; // resumes/continues timer/counter
        
        // prevents OCR3A from overflowing, using prescaler 64
        // 0.954 is smallest frequency that will not result in overflow
        if (frequency < 0.954) OCR3A = 0xFFFF;
        
        // prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
        else if (frequency > 31250) OCR3A = 0x0000;
        
        // set OCR3A based on desired frequency
        else OCR3A = (short)(8000000 / (128 * frequency)) - 1;
        
        TCNT3 = 0; // resets counter
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    // COM3A0: Toggle PB6 on compare match between counter and OCR3A
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    // WGM32: When counter (TCNT3) matches OCR3A, reset counter
    // CS31 & CS30: Set a prescaler of 64
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

enum Song_States{song_start, Play} song_state;

#define Gs2 103.83
#define Gs3 207.65
#define Gs4 415.30

#define C2	65.41
#define Db2	69.30
#define D2	73.42
#define Eb2	77.78
#define E2	82.41
#define F2	87.31
#define Gb2	92.50
#define G2	98.00
#define Ab2	103.83
#define A2	110.00
#define Bb2	116.54
#define B2	123.47

#define C3	130.81
#define Db3	138.59
#define D3	146.83
#define Eb3	155.56
#define E3	164.81
#define F3	174.61
#define Gb3	185.00
#define G3	196.00
#define Ab3	207.65
#define A3	220.00
#define Bb3	233.08
#define B3	246.94

#define C4	261.94
#define Db4	277.18
#define D4	293.66
#define Eb4	311.13
#define E4	329.63
#define F4	349.23
#define Gb4	369.99
#define G4	392.00
#define Ab4	415.30
#define A4	440.00
#define Bb4	466.16
#define B4	493.88

#define C5	523.25
#define Db5	554.37
#define D5	587.33
#define Eb5	622.25
#define E5	657.25
#define F5	698.46
#define Gb5	739.99
#define G5	783.99
#define Ab5	830.61
#define A5	880.00
#define Bb5	932.33
#define B5	987.77

double notes[] = {E5, B4, C5, D5, C5, B4, A4, A4, C5, E5, D5, C5, B4, C5, D5, E5, C5, A4, A4, 0, D5, F5, A5, G5, F5, E5, C5, E5, D5, C5, B4, B4, C5, D5, E5, C5, A4, A4, 0,
    E5, B4, C5, D5, C5, B4, A4, A4, C5, E5, D5, C5, B4, C5, D5, E5, C5, A4, A4, 0, D5, F5, A5, G5, F5, E5, C5, E5, D5, C5, B4, B4, C5, D5, E5, C5, A4, A4, 0,
    E3, C3, D3, B2, C3, A2, Gs2, 0, E3, C3, D3, B2, C3, E3, A3, Gs3, 0};
unsigned short timing[] = {40, 20, 20, 40, 20, 20, 40, 20, 20, 40, 20, 20, 60, 20, 40, 40, 40, 40, 40, 60, 40, 20, 40, 20, 20, 60, 20, 40, 20, 20, 40, 20, 20, 40, 40, 40, 40, 40, 40,
    40, 20, 20, 40, 20, 20, 40, 20, 20, 40, 20, 20, 60, 20, 40, 40, 40, 40, 40, 60, 40, 20, 40, 20, 20, 60, 20, 40, 20, 20, 40, 20, 20, 40, 40, 40, 40, 40, 40,
    80, 80, 80, 80, 80, 80, 160, 20, 80, 80, 80, 80, 40, 40, 80, 160, 40};
unsigned char curr_note;
unsigned char num_notes = 95;
signed short cnt;

void Song_Tick(){
    switch(song_state) {
        case song_start:
            curr_note = 0;
            cnt = 0;
            song_state = Play;
            break;
        case Play:
            if(cnt < 0) {
                set_PWM(0);
                cnt++;
                break;
            }
            set_PWM(notes[curr_note]);
            cnt++;
            if(cnt >= timing[curr_note]) {
                cnt = -1;
                curr_note = (curr_note + 1) % num_notes;
            }
    }
}
//End song state

//Button Press //task3
unsigned char buttonC;
unsigned char tmpB;
unsigned char temp = 0x00;
unsigned long delay = 0;
unsigned char game_start = 0;
enum Button_States{button_start, release, press} button_state;
void Button_Tick(){
    switch(button_state){
        case button_start:
            button_state = release;
            break;
        case release:
            if(buttonC == 0x01){
                USART_Send(0xFF,1);
                button_state = press;
            }
            else{
                button_state = release;
            }
            break;
        case press:
            if(buttonC == 0x01){
                USART_Send(0xFF, 1);
                button_state = press;
            }
            else{

                button_state = release;
                USART_Flush(1);
            }
            break;
    }
}
//End of Button

//Game
unsigned char store; //0x01
unsigned char spin; //0x02
unsigned char left; // 0x04
unsigned char right; //0x10
unsigned char down; //0x08
unsigned char game_over;
unsigned char game_button;
//unsigned char commands[1];
//unsigned char tme = 0;
enum Game_States{gamestart} game_state;
 void Game_tick()
 {
	switch(game_state){
		case gamestart:		
			//store
			if(game_button == 0x01){
				sendArduino(0x40);			
			}
			//spin
			else if(game_button == 0x02){
				sendArduino(0x41);
			}
			//left
			else if(game_button == 0x04){
				sendArduino(0x42);
			}
			//right
			else if(game_button == 0x10){
				sendArduino(0x43);
			}
			//down
			else if(game_button == 0x08){
				sendArduino(0x44);
			}
			game_state = gamestart;
			break;
			
	}
 }
 
 void transmit_data(unsigned char data){
	 int i;
	 for(i = 7; i >= 0; --i){
		 PORTB = 0x08;
		 PORTB = PORTB | ((data >> i) & 0x01);
		 PORTB = PORTB | 0x04;
	 }
	 PORTB = PORTB | 0x02;
	 PORTB = 0x00;
 }
 
 unsigned char score;
 unsigned char count = 0;
 enum Score_States{score_start, score_out, inc_score_press, inc_score_release} score_state;
void Score_Tick(){
	switch(score_state){
		case score_start:
			if(score == 0x40){
				score_state = inc_score_press;
			}
			else{
				score_state = score_out;
			}
			break;
		case score_out:
			if(score == 0x40){
				score_state = inc_score_press;
			}
			else{
				if(count == 0){
					transmit_data(0x40);
				}
				else if(count == 1){
					transmit_data(0x79);
				}
				else if(count == 2){
					transmit_data(0x24);
				}
				else if(count == 3){
					transmit_data(0x30);
				}
				else if(count == 4){
					transmit_data(0x19);
				}
				else if(count == 5){
					transmit_data(0x12);
				}
				else if(count == 6){
					transmit_data(0x02);
				}
				else if(count == 7){
					transmit_data(0x78);
				}
				else if(count == 8){
					transmit_data(0x00);
				}
				else if(count == 9){
					transmit_data(0x18);
				}
				score_state = score_out;
			}
			break;
		case inc_score_press:
			if(score == 0x40){
				score_state = inc_score_press;
			}
			else{
				score_state = inc_score_release;
			}
			break;
		case inc_score_release:
			if(count < 9){
				count++;
			}
			USART_Send(0xFA, 1);
			score_state = score_out;
			break;
	}
};

int main()
{
    DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x00; PORTC = 0xFF;
    DDRD = 0xFF; PORTD = 0x00;
    /*
     static task task1, task2, task3;
     task *tasks[] = {&task1, &task2, &task3};
     const unsigned short numTasks =	3;
     
     task1.state = LCD_start;
     task1.period = 25;
     task1.elapsedTime = 25;
     task1.TickFct = &LCD_Tick;
     
     task2.state = song_start;
     task2.period = 10;
     task2.elapsedTime = 10;
     task2.TickFct = &Song_Tick;
     
     task3.state = button_start;
     task3.period = 5;
     task3.elapsedTime = 5;
     task3.TickFct = &Button_Tick;
     */
    PWM_on();
    LCD_init();
    TimerSet(10);
    TimerOn();
    initUSART(0);
	initUSART(1);
    
    //unsigned short i;
    while(1) {
        buttonC = ~PINC & 0xFF;
        PORTB = 0x00;
        temp = 0x00;
		//USART_Flush(0);
        /*
         for ( i = 0; i < numTasks; i++ ) {
         if ( tasks[i]->elapsedTime == tasks[i]->period ) {
         tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
         tasks[i]->elapsedTime = 0;
         }
         tasks[i]->elapsedTime += 1;
         }
         */
        Button_Tick();
        LCD_Tick();
		Song_Tick();
		Score_Tick();
        
        if(USART_HasReceived(1)){
            temp = USART_Receive(1);
        }
        if(temp == 0xFF && buttonC == 0x01){
            game_start = 1;
        }
        while(game_start){
            if(delay <= 100){
                LCD_DisplayString(1, "Ready...");
				Song_Tick();
            }
            else if(delay > 100 && delay <= 200){
                LCD_DisplayString(1, "3");
				Song_Tick();
            }
            else if(delay > 200 && delay <= 300){
                LCD_DisplayString(1, "2");
				Song_Tick();
			}
            else if(delay > 300 && delay <= 400){
                LCD_DisplayString(1, "1");
				Song_Tick();
            }
            else if(delay > 400 && delay <= 500){
                LCD_DisplayString(1, "Go!");
				sendArduino(0xFF);
				Song_Tick();
            }
            else{
				USART_Flush(0);
				LCD_ClearScreen();
				game_over = 0;
				while(!game_over)
				{
					game_button = ~PINC & 0x1F;
					score = ~PINC & 0x60;
					Song_Tick();
					Game_tick();
					/*
					if(USART_HasReceived(0)){
						temp = USART_Receive(0);
						if(temp == 0x23){
							USART_Send(0x23,1);
						}
					}
					if(USART_HasReceived(1)){
						temp = USART_Receive(1);
						if(temp == 0x23){
							USART_Send(0x23, 0);
						}
					}
					*/
					if(USART_HasReceived(0))
					{
						temp = USART_Receive(0);
						if(temp == 0x21)
						{
							LCD_DisplayString(1,"Loser");
							count = 0;
							USART_Send(0x21,1);
							game_over = 0x01;
						}
					}
					if(USART_HasReceived(1))
					{
						temp = USART_Receive(1);
						if(temp == 0x21)
						{
							count++;
							LCD_DisplayString(1, "Winner!");
							sendArduino(0x21);
							game_over = 0x01;
						}
					}
					while(!TimerFlag){};
					TimerFlag = 0;
				}
				delay = 0;
				game_start = 0;
            }
            delay++;
            while(!TimerFlag){};
            TimerFlag = 0;
            
        }
        while(!TimerFlag){};
        TimerFlag = 0;
    }
    return 0;
}
