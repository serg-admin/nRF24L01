#include "spi.h"
#include "uart_async.h"

volatile uint8_t *devSSport;
uint8_t devSSpin;

/**
 * @brief Ожидает освобождение SPI шины и захватывает ее
 * @param port Порт для пробуждения SS устройства
 * @param pin  Пин для пробуждения SS устройства
 */
void spiGetBus(volatile uint8_t *port, uint8_t pin) {
  while (spi_script.status) sleep_mode();
  cli();
  spi_script.status = SPI_STATE_BUSY;
  sei();
  *port &= ~(_BV(pin));
  devSSport = port;
  devSSpin  = pin;
}

/**
 * @brief Отмечает SPI шину как свободную
 */
void spiSetFree(void) {
  *devSSport |= _BV(devSSpin);
  spi_script.status = SPI_STATE_FREE;
}

uint8_t spiTransmit(void) {
  spi_script.pos = 0;
  SPDR = spi_script.script[spi_script.pos++];
  return 0;
}

void spi_masterInit(void) {
  /* Set MOSI and SCK output, all others input */
  DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK) | _BV(DD_SCN) | _BV(DD_MISO);
  PORT_SPI |= _BV(PIN_SCN); // Переводим SPI Slave в сон.
  /* Enable SPI, Master, set clock rate fck/16 */
  SPCR = _BV(SPE)|(1<<MSTR)|(1<<SPR0) | _BV(SPIE); // Включаем прерывания
  spi_script.status = SPI_STATE_FREE;
}

ISR (SPI_STC_vect) {
  spi_script.script[spi_script.pos - 1] = SPDR;
  if (spi_script.pos < spi_script.size) {
    SPDR = spi_script.script[spi_script.pos++];
  } else {
    spi_script.callBack(spi_script.script, spi_script.size);
  }
}