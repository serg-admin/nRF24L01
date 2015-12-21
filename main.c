//#define F_CPU 16000000
//#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include "tools/timer16.h"
#include "tools/uart_async.h"
#include "tools/error.h"
//#include "tools/pcint.h"
#include "tools/spi.h"
#include "tools/eeprom.h"

#if defined (__AVR_ATmega128__)
#  define NRF24L01_SCN_DDR DDRB
#  define NRF24L01_SCN_PORT PORTB
#  define NRF24L01_SCN_DDN DDB0
#  define NRF24L01_SCN_PIN PORTB0
#  define NRF24L01_CE_DDR DDRB
#  define NRF24L01_CE_PORT PORTB
#  define NRF24L01_CE_PIN PORTB3
#  define NRF24L01_CE_DDN DDB3
#else
#  define NRF24L01_SCN_DDR DDRB
#  define NRF24L01_SCN_PORT PORTB
#  define NRF24L01_SCN_DDN DDB2
#  define NRF24L01_SCN_PIN PORTB2
#  define NRF24L01_CE_DDR DDRB
#  define NRF24L01_CE_PORT PORTB
#  define NRF24L01_CE_PIN PORTB1
#  define NRF24L01_CE_DDN DDB1
#endif

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
  while(str[pos*2] != 0) {
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
  uart_writelnHEXEx(buf, size);
 // spiData.status = SPI_STATE_FREE;
  return 0;  
}

void nRF24L01ClearSCN(uint8_t* args) { 
  NRF24L01_CE_PORT &= ~(_BV(NRF24L01_CE_PIN));  
}

uint8_t nRF24L01SendActivate(uint8_t *buff, uint8_t size) {
  NRF24L01_CE_PORT |= _BV(NRF24L01_CE_PIN);  
  timer1PutTask(1, &nRF24L01ClearSCN, 0);
  
  //spiData.status = SPI_STATE_FREE;
  return 0;
}

void nRF24L01SendStr(char *str) {  
  struct rec_spi_data *data; 
  //NRF24L01_CE_PORT |= _BV(NRF24L01_CE_PIN);
  data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendBuff[0] = 0b10100000; // Запись в TX буффер
  data->sendSize = 0;
  while(str[data->sendSize]) {
    data->sendBuff[data->sendSize+1] = str[data->sendSize];
    data->sendSize++;
  }
  data->sendSize++;
  data->callBack = &nRF24L01SendActivate;
  spiTransmit(0);
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
  uint8_t tmp_arr[2];
  struct rec_spi_data *data;
  if ((str[0] == 'S') && (str[1] == 'P') && (str[2] == 'I')) {
    if (spiData.status) {
      uart_writeln("SPI busy");
      return;
    }
    data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
    data->sendSize = parse_HEX_string(str + 3, data->sendBuff); 
    if (! spiData.sendSize) {
      _log(ERR_COM_PARSER_PARS_ERR);
      spiSetFree();
      return;
    }
    data->callBack = &writelnHEX;
    spiTransmit(data->sendSize);
    return;
  } 
  if ((str[0] == 'S') && (str[1] == 'E') && (str[2] == 'N') && (str[3] == 'D')) {
    nRF24L01SendStr(&str[4]);
    return;
  } 
  if ((str[0] == 'R') && (str[1] == 'O') && (str[2] == 'M') && (str[3] == 'W')) {
    parse_HEX_string(str + 4, tmp_arr);
    if (EEPROM_write(tmp_arr[0] * 0xFF + tmp_arr[1], tmp_arr[2])) 
      uart_writeln("eeprom busy");
    return;
  } 
  if ((str[0] == 'R') && (str[1] == 'O') && (str[2] == 'M') && (str[3] == 'R')) {
    parse_HEX_string(str + 4, tmp_arr);
    uart_writelnHEX(EEPROM_read(tmp_arr[0] * 0xFF + tmp_arr[1]));
    return;
  } 
  _log(ERR_COM_PARSER_UNKNOW_COM);
}

void nRF24L01_init(void) {
  struct rec_spi_data *data;
  // Установка адреса для дефолтного канала приема 
  // RX_ADDR_P0 - Receive address data pipe 0. 5 Bytes maximum 
  // length. (LSByte is written first. Write the number of 
  // bytes defined by SETUP_AW).
  data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendSize = parse_HEX_string("2A0000000002", data->sendBuff);
  spiTransmit(0);
  
  data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendSize = parse_HEX_string("2B0000000002", data->sendBuff);
  spiTransmit(0);
  
  // Установка адреса для дефолтного канала передачи 
  // TX_ADDR - Transmit address. Used for a PTX device only. 
  // (LSByte is written first) Set RX_ADDR_P0 equal to this address 
  // to handle automatic acknowledge if this is a PTX device with 
  // Enhanced ShockBurst™ enabled
  data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendSize = parse_HEX_string("300000000002", data->sendBuff);
  spiTransmit(0);
  
  // Включаем трансивер
  data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendBuff[0] = 0x20;
  data->sendBuff[1] = 0x0A;
  data->sendSize = 2;
  spiTransmit(0);
}

int main(void) {
  uint8_t spi_read_buff[32];
  timer_init();
  uart_async_init();
  //pcint_init(0);
  uart_readln(&commands_reciver);
  sei();
  uart_writeln("Start");
  spi_init(spi_read_buff);
  NRF24L01_SCN_DDR |= _BV(NRF24L01_SCN_DDN);
  NRF24L01_SCN_PORT |= _BV(NRF24L01_SCN_PIN);
  NRF24L01_CE_DDR |= _BV(NRF24L01_CE_DDN);
  NRF24L01_CE_PORT &= ~(_BV(NRF24L01_CE_PIN));
  nRF24L01_init();
  while(1) {
    sleep_mode();
  }
  return 0;
}
