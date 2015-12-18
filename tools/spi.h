#ifndef __SPI_H_
#define __SPI_H_
#define __AVR_ATmega328P__ 1
#include <avr/interrupt.h>
#include <avr/io.h>

#define PORT_SPI PORTB
#define DDR_SPI DDRB
#define DD_CE   DDB0
#define DD_IRQ  DDB1
#define DD_SCN  DDB2  // SS
#define PIN_SCN PORTB2
#define DD_MOSI DDB3
#define DD_MISO DDB4
#define DD_SCK  DDB5

#define SPI_SCRIPT_LEN 32
#define SPI_STATE_FREE 0
#define SPI_STATE_BUSY 1

struct rec_isp_script {
  uint8_t script[SPI_SCRIPT_LEN];
  uint8_t size;
  uint8_t pos;
  uint8_t status;
  uint8_t (* callBack)(uint8_t* buf, uint8_t size);
} spi_script;

uint8_t spi_transmit(void);
void spi_masterInit(void);
void spiWakeup(volatile uint8_t *port, uint8_t pin);
void spiSleep(volatile uint8_t *port, uint8_t pin);

#endif
