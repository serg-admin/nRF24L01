#ifndef __TIMER16_H_
#define __TIMER16_H_ 1
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "uart_async.h"
//Настройки магающей лампочки
#define ledOn PORTD |= _BV(PIND5)  
#define ledOff PORTD &= ~(_BV(PIND5))
#define ledSw PORTD ^= _BV(PIND5)
uint16_t timer16_start_value;

void timer_init();
void timer1PutTask(uint16_t delay, void (*func)(uint8_t*), uint8_t* data);

#endif