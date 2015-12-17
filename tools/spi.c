#include "spi.h"

uint8_t spi_transmit(void) {
  if (spi_script.status > 0) return spi_script.status;
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
  spi_script.status = ISP_STATE_FREE;
}

ISR (SPI_STC_vect) {
  spi_script.script[spi_script.pos - 1] = SPDR;
  if (spi_script.pos < spi_script.size) {
    SPDR = spi_script.script[spi_script.pos++];
  } else {
    spi_script.callBack(spi_script.script, spi_script.size);
  }
}