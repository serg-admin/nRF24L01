#include "pcint.h"

ISR (PCINT0_vect) {
  cli();
  uart_writeln("INT0");
  if (pcintCallBack) pcintCallBack();
  sei();
}

void pcint_init(uint8_t * f) {
  PCICR  |= _BV(PCIE0);
  PCMSK0 |= _BV(PCINT1);
  PORTB  |= _BV(PORTB1);
  pcintCallBack = 0;
}