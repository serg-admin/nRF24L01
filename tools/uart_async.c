#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "uart_async.h"

unsigned char uart_buf[UART0_BUFER_SIZE];
char uart_read_buf[UART0_READ_BUFER_SIZE];
unsigned char uart_read_wPos = 0;
unsigned char uart_wPos = 0; // Позиция буфера для записи новых данных.
unsigned char uart_rPos = 0; // Позиция буфера для передачи в порт.
void (*uart_readln_callback)(char*) = 0;


ISR (USART_UDRE_vect) {
  cli();
  if (uart_wPos != uart_rPos) {
    UDR0 = uart_buf[uart_rPos++];
    if (uart_rPos >= UART0_BUFER_SIZE) uart_rPos = 0;
  } else UCSR0B &= ~(_BV(UDRIE0));
  sei();
}

// Прерывание - Пришел байт данных.
ISR (USART_RX_vect) {
  PORTB ^= _BV(PINB5);
  if (uart_readln_callback == 0) return;
  cli();
  while (UCSR0A & _BV(RXC0)) {
    unsigned char t = UDR0; // Из порта байт можно прочитать только один раз.
    if ((t == 0x0A) || (t == 0x0D)) {
      uart_read_buf[uart_read_wPos] = 0; //Конец строки
      uart_readln_callback(uart_read_buf);
      uart_read_wPos = 0;
    } else {
      uart_read_buf[uart_read_wPos++] = t;
    }
  }
  sei();
}

void uart_readln(void (*callback)(char*)) {
  uart_readln_callback = callback;
}

void uart_async_init(void) {
   uart_wPos = 0;
   uart_rPos = 0;
   // Разрешить прием, передачу через порт.
   UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0) ;
   // Устанавливаем скорость порта.
   UBRR0 = MYBDIV;
   UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

// Возвращает количество свободных байт в очереди USART.
char uart_getBufSpace() {
  if (uart_rPos <= uart_wPos) {
    return UART0_BUFER_SIZE - uart_wPos + uart_rPos - 1;
  } else {
    return uart_rPos - uart_wPos - 1;
  }
}

// Отправляет один байт в очередь USART. В случае если очередь занята - ждет.
void uart_putChar(char c) {
  if (uart_getBufSpace() == 0) return;
  UCSR0B &= ~(_BV(UDRIE0));
  uart_buf[uart_wPos++] = c;
  if (uart_wPos >= UART0_BUFER_SIZE) uart_wPos = 0;
  if (UCSR0A & _BV(UDRE0)){
    if (uart_wPos != uart_rPos) {
      UDR0 = uart_buf[uart_rPos++];
      if (uart_rPos >= UART0_BUFER_SIZE) uart_rPos = 0;
    }
  }
  UCSR0B |= _BV(UDRIE0);
}



// Отправляет 0 терменированную строку в очередь USART.
void uart_write(char* s) {
  unsigned char i = 0;
  while(s[i] != 0) {
    uart_putChar(s[i++]);
  }
}

// Отправляет строку в очередь USART. В случае если очередь занята - ждет.
void uart_writeln(char* s) {
  uart_write(s);
  uart_write("\n\r");
}

char uart_halfchar_to_hex(unsigned char c) {
  if (c < 10) return c + 48;
  switch (c) {
    case 10 : return 'A';
    case 11 : return 'B';
    case 12 : return 'C';
    case 13 : return 'D';
    case 14 : return 'E';
    case 15 : return 'F';
  }
  return 0;
}

void _uart_writeHEX(unsigned char c) {
  uart_putChar(uart_halfchar_to_hex(c >> 4));
  uart_putChar(uart_halfchar_to_hex(c & 0x0F));
}

void _log(uint16_t code) {
  uart_write("E-");
  _uart_writeHEX(((unsigned char*)&code)[1]);
  _uart_writeHEX(((unsigned char*)&code)[0]);
  uart_writeln("");
}

void uart_writelnHEXEx(unsigned char* c, unsigned char size) {
  unsigned char i;
  uart_write("0x");
  for(i=0; i < size; i++) {
    _uart_writeHEX(c[i]);
    uart_putChar(' ');
  }
  uart_writeln("");
}

void uart_writelnHEX(unsigned char c) {
  uart_writelnHEXEx(&c, 1);
}
