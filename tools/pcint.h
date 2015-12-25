#ifndef __PCINT_H_
#define __PCINT_H_ `
#include <avr/interrupt.h>
#include "uart_async.h"

typedef void _pcintCallBack(void);
_pcintCallBack* pcintCallBack;

void pcint_init(uint8_t * f);

#endif