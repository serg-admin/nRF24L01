#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "tools/timer16.h"
#include "tools/uart_async.h"
#include "tools/error.h"
#include "tools/pcint.h"
#include "tools/spi.h"

#define NRF24L01_SCN_DDR DDRB
#define NRF24L01_SCN_PORT PORTB
#define NRF24L01_SCN_PIN DDB2
#define NRF24L01_CE_DDR DDRB
#define NRF24L01_CE_PORT PORTB
#define NRF24L01_CE_PIN DDB0

unsigned char hexToCharOne(char c) {
  if ((c > 47) && (c<58)) return c-48;
  switch(c) {
    case 'A' : return 0x0A;
    case 'B' : return 0x0B;
    case 'C' : return 0x0C;
    case 'D' : return 0x0D;
    case 'E' : return 0x0E;
    case 'F' : return 0x0F;
    default : return 0xFF;
  }
}

/**
 * @brief Преобразует строку HEX символов (0-F) в массив байт.
 * @param str  строка HEX символов
 * @param result  указатель на буфер для сохранения результата.
 * @return возвращает длинну полученного массива.
 */
uint8_t parse_HEX_string(char* str, uint8_t* result) {
  uint8_t pos = 0;
  uint8_t tmp;
  while((str[pos*2] != 0) && 
          (pos < SPI_SCRIPT_LEN)) {
      result[pos] = hexToCharOne(str[pos*2]);
      tmp = hexToCharOne(str[pos*2+1]);
      if ((tmp & 0xF0) || (result[pos] & 0xF0)) {
        return 0;
      }
      result[pos] <<= 4;
      result[pos] |= tmp;
      pos++;
    }
  return pos;
}

uint8_t writelnHEX(uint8_t* buf, uint8_t size) {
  spiSleep(&PORT_SPI, PIN_SCN);
  uart_writelnHEXEx(buf, size);
  spi_script.status = SPI_STATE_FREE;
  return 0;  
}

/**
 * @brief Разбирает строковый сценарий и отправляет его на I2C шину
 * @param str Сценрий в ввиде IS1A1D1D2D3...S2A2D4D5D6...
 *    I              - префикс
 *    S1, S2,...,Sn - Размер текущего блока;
 *    A1, A1,...,An - Адрес на шине I2C с флагом чтение/запись;
 *    D1, D2,...,Dn - Отправляемые данные, или ожидаемый объем принимаемых данных.
 *  Например I02D00002D107 - отправить байт 00 на устройство D0, прочитать 7-мь байт 
 *  с устройства D1.
 */
void commands_reciver(char* str) {
  if (str[0] == 'S') {
    cli();
    if (spi_script.status) {
      _log(ERR_SPI_BUSY);
      sei();
      return;
    }
    spi_script.status = SPI_STATE_BUSY;
    sei();
    spi_script.size = parse_HEX_string(str + 1, spi_script.script);
    if (! spi_script.size) {
      _log(ERR_COM_PARSER_PARS_ERR);
      spi_script.status = SPI_STATE_FREE;
      return;
    }
    spiWakeup(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
    spi_script.callBack = &writelnHEX;
    spi_transmit();
  }
}

uint8_t nRF24L01_initCallBack(uint8_t* arg, uint8_t size) {
  spi_script.status = SPI_STATE_FREE;
  uart_writeln("free");
  spiSleep(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  return 0;
}

void nRF24L01_init(void) {
  while (spi_script.status) sleep_mode();
  cli();
  spi_script.status = SPI_STATE_BUSY;
  sei();
  spiWakeup(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  // Установка адреса для дефолтного канала приема 
  // RX_ADDR_P0 - Receive address data pipe 0. 5 Bytes maximum 
  // length. (LSByte is written first. Write the number of 
  // bytes defined by SETUP_AW).
  spi_script.size = parse_HEX_string("2A0000000002", spi_script.script);
  spi_script.callBack = &nRF24L01_initCallBack;
  uart_write("begin1 = "); uart_writelnHEX(spi_script.size);
  spi_transmit();
  while (spi_script.status) sleep_mode();
  uart_write("end1");
  cli();
  spi_script.status = SPI_STATE_BUSY;
  sei();
  spiWakeup(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  // Установка адреса для дефолтного канала передачи 
  // TX_ADDR - Transmit address. Used for a PTX device only. 
  // (LSByte is written first) Set RX_ADDR_P0 equal to this address 
  // to handle automatic acknowledge if this is a PTX device with 
  // Enhanced ShockBurst™ enabled
  spi_script.size = parse_HEX_string("300000000002", spi_script.script);
  spi_script.callBack = &nRF24L01_initCallBack;
  spi_transmit();
}

int main(void) {
  timer_init();
  uart_async_init();
  spi_masterInit();
  pcint_init(0);
  uart_readln(&commands_reciver);
  sei();
  uart_writeln("Start");
  DDR_SPI |= _BV(DD_SCN);
  PORT_SPI |= _BV(PIN_SCN);
  nRF24L01_init();
  while(1) {
    sleep_mode();
  }
  return 0;
}
