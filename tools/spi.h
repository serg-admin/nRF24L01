#ifndef __SPI_H_
#define __SPI_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "uart_async.h"

#define PORT_SPI PORTB
#define DDR_SPI DDRB

#if defined (__AVR_ATmega128__)
#  define DD_SS   DDB2
#  define DD_MOSI DDB2
#  define DD_MISO DDB3
#  define DD_SCK  DDB1
#else
#  define DD_SS   DDB2
#  define DD_MOSI DDB3
#  define DD_MISO DDB4
#  define DD_SCK  DDB5
#endif

#define SPI_STATE_FREE 0
#define SPI_STATE_BUSY 1

/* Максимальный размер буфера для приема/передачи данных SPI шины. */
uint8_t spiReciveBuffSize;

/**
 * @brief rec_isp_script структура приема/передачи данных шины SPI.
 * @param sendBuff Хранит передаваемые данных SPI шины.
 * @param reciveBuff Хранит принимаемые данных SPI шины.
 * @param sendSize Размер передаваемых данных.
 * @param reciveSize Размер принимаемых данных.
 * @param pos Текущая позиция буферов.
 * @param status Состояние SPI шины (SPI_STATE_FREE, SPI_STATE_BUSY ...) - 0 если свободно
 * @param callBack функция которая будет вызвана по завершению передачи.
 */
struct rec_spi_data {
  uint8_t* sendBuff;
  uint8_t* reciveBuff;
  uint8_t sendSize;
  uint8_t reciveSize;
  uint8_t pos;
  uint8_t status;
  uint8_t (* callBack)(uint8_t* buf, uint8_t size);
} spiData;

/**
 * @brief Инициализирует выводы шины SPI, должна вызываться через макрос SPI_INIT(size).
 * 
 */
void spi_init(uint8_t *buff);

/**
 * @brief Ожидает освобождение SPI шины и захватывает ее
 * @param port Порт для пробуждения SS устройства
 * @param pin  Пин для пробуждения SS устройства
 */
struct rec_spi_data* spiGetBus(volatile uint8_t *port, uint8_t pin);

/**
 * @brief Начинает трансляцию данных через SPI шину.
 * @param recive_size Размер принимаемых данных.
 * @return 0 при коректном старте передачи.
 */
uint8_t spiTransmit(uint8_t recive_size);

/**
 * @brief Отмечает SPI шину как свободную и  устанавливает SS вывод для активного устройства в 1.
 */
void spiSetFree(void);

#endif
