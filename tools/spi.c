#include "spi.h"

/* Данные для дизактивации текущего устрйства */
volatile uint8_t *devSSport;
uint8_t devSSpin;

struct rec_spi_data* spiGetBus(volatile uint8_t *port, uint8_t pin) {
  while (spiData.status) sleep_mode();
  spiData.status = SPI_STATE_BUSY;
  *port &= ~(_BV(pin));
  devSSport = port;
  devSSpin  = pin;
  spiData.callBack = 0;
  return &spiData;
}

/**
 * @brief Отмечает SPI шину как свободную
 */
void spiSetFree(void) {
  *devSSport |= _BV(devSSpin);
  spiData.status = SPI_STATE_FREE;
  spiData.callBack = 0;
  spiData.sendBuff = spiData.reciveBuff;
}

uint8_t spiTransmit(uint8_t recive_size) {
  SPDR = spiData.sendBuff[0];
  spiData.pos = 1;
  spiData.reciveSize = recive_size;
  return 0;
}

void spi_init(uint8_t *buff) {
  // Шина свободна
  spiData.callBack = 0;
  spiData.status = SPI_STATE_FREE;
  spiData.reciveBuff = buff;
  spiData.sendBuff = spiData.reciveBuff;
  /* MOSI, MISO  и SCK как выходы */
  DDR_SPI |= _BV(DD_MOSI) | _BV(DD_SCK) | _BV(DD_SS);//| _BV(DD_MISO);
  /* Включаем SPI как мастер с частотой fck/16. Активируем прерывание SPI */
  SPCR = _BV(SPE) | _BV(MSTR) | (1<<SPR0) | _BV(SPIE); 
}

ISR (SPI_STC_vect) {
//void SIG_SPI( void ) __attribute__ ( ( signal, naked ) );
//void SPI_STC_vect( void ) {
  // Збрасываем флаг прерывания
  uint8_t tmp = SPDR;
  cli();
  if (spiData.status != SPI_STATE_FREE) {
    if ((spiData.pos - 1) < spiData.reciveSize)
      spiData.reciveBuff[spiData.pos - 1] = tmp;
    if (spiData.pos < spiData.sendSize) {
      SPDR = spiData.sendBuff[spiData.pos++];
    } else {
      // Размер принимаемых данных больше передаваемых
      // Для генерации тактов SCK передаем данные
      if (spiData.pos < spiData.reciveSize) {
        SPDR = 0;
      } else {
        if (spiData.callBack)
          spiData.callBack(spiData.reciveBuff, spiData.pos);
        spiSetFree();
      }
    }
  }
  sei();
}