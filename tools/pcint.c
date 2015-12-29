#include "pcint.h"

#if defined (__AVR_ATmega128__)
ISR (INT0_vect) {
#elif defined (__AVR_ATmega8515__)
ISR (INT0_vect) {
#else
ISR (PCINT0_vect) {
#endif
  cli();
  //uart_writeln("INT0");
  sei();
  if (pcintCallBack) pcintCallBack();
}

void pcint_init(uint8_t * f) {
  pcintCallBack = 0;
#if defined (__AVR_ATmega128__)
  PORTD  |= _BV(PORTD0);
  EICRA |= _BV(ISC01) ; //Срабатывать по спадающему краю
  EIMSK |= _BV(INT0);
#elif defined (__AVR_ATmega8515__)
  PORTD  |= _BV(PORTD2);
  MCUCR |= _BV(ISC01) ; //Срабатывать по спадающему краю
  GICR |= _BV(INT0);
#else
  PORTB  |= _BV(PORTB0);
  PCMSK0 |= _BV(PCINT0);
  PCICR  |= _BV(PCIE0);
#endif
}