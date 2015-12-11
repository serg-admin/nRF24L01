#include <avr/io.h>
#include <tools/timer16.h>
#include <tools/uart_async.h>
#define PORT_SPI PORTB
#define DDR_SPI DDRB
#define DD_CE   DDB0
#define DD_IRQ  DDB1
#define DD_SCN  DDB2  // SS
#define PIN_SCN PORTB2
#define DD_MOSI DDB3
#define DD_MISO DDB4
#define DD_SCK  DDB5

void pcint_init(uint8_t * f) {
  EIMSK  |= _BV(INT0);
  PCMSK0 |= _BV(PCINT1);
  uart_writeln("1");
}

ISR (PCINT0_vect) {
  cli();
  PCMSK0 &= ~(_BV(PCINT1));
  uart_writeln("INT0");
  timer1PutTask(50000, &pcint_init, 0);
  sei();
}

void SPI_MasterInit(void) {
  /* Set MOSI and SCK output, all others input */
  DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK);
  /* Enable SPI, Master, set clock rate fck/16 */
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
  
  pcint_init(0);
}

void SPI_MasterTransmit(char cData) {
  /* Start transmission */
  PORT_SPI &= ~(_BV(PIN_SCN));
  SPDR = cData;
  /* Wait for transmission complete */
  while (!(SPSR & (1<<SPIF)));
  PORT_SPI |= _BV(PIN_SCN);
}

void nRF24L01_init(void) {
  SPI_MasterTransmit(0x88);
}

int main(void) {
  timer_init();
  uart_async_init();
  SPI_MasterInit();
  sei();
  uart_writeln("Start");
  nRF24L01_init();
  sleep_enable();
  while(1) {
    sleep_mode();
  }
	return 0;
}
