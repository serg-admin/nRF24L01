//#define F_CPU 16000000
//#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include "tools/timer16.h"
#include "tools/uart_async.h"
#include "tools/error.h"
#include "tools/pcint.h"
#include "tools/spi.h"
#include "tools/eeprom.h"
#include "tools/nRf24l01.h"

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
  return 0;  
}

uint8_t nRF24L01Status(uint8_t* buff, uint8_t size) {
  struct rec_spi_data *data;
  switch (nRF24L01State) {
    case 0x61 : // Был запрошен буфер приема (RX)
      // Обработка полученного блока данных - структура блока:
      // 0x00 - Размер значащих данных (включая байт 0x00).
      // 0x01 - Функция обработки данных.
      // .... - Данные.
      for(nRF24L01Data.dataSize = 0; nRF24L01Data.dataSize < buff[0] ;nRF24L01Data.dataSize++)
        ((uint8_t*)&nRF24L01Data)[nRF24L01Data.dataSize] = buff[nRF24L01Data.dataSize];
      nRF24L01Data.dataSize--;   nRF24L01Data.dataSize--;
      // Проверка сотояния буферов
      data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
      data->sendBuff = &nRF24L01State;
      data->sendBuff[0] = 0x17;
      spiTransmit(2);
      sei();
      case (nRF24L01Status) 
      break;
    case 0xFF : // Был запрошен регистр статуса (вход для IRQ)
      if (buff[0] & 0b01000000) { // Есть данные в буфере приема (RX).
        data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
        data->sendBuff = &nRF24L01State;
        data->sendBuff[0] = 0x61;
        spiTransmit(nRF24L01_conf.rx_pw_p1);
      }
      break;
  }
  return 0;
}

void nRF24L01IRQ(void) {
  struct rec_spi_data *data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendBuff = &nRF24L01State;
  data->sendBuff[0] = 0xFF; // Пустая команда для получения статуса
  data->reciveSize = 1;
  data->callBack = &nRF24L01Status;
  spiTransmit(1);
}

uint8_t nRF24L01Sending(uint8_t* buff, uint8_t size) {
  // Преводим в режим передачи
  nRF24L01_SET_SEND;
  return 0;
}

void nRF24L01Send(uint8_t* buff, uint8_t size) {
  uint8_t i;
  struct rec_spi_data *data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  // Команда отправить данные
  data->reciveBuff[0] = 0xA0;
  // Фактический размер значащих данных
  data->reciveBuff[1] = size;
  for(i = 0; i < size; i++) {
    data->sendBuff[i+2] = buff[i];
  }
  // Размер окна данных в пакете
  data->sendSize = nRF24L01_conf.rx_pw_p0;
  data->callBack = &nRF24L01Sending;
  spiTransmit(0);
}

void nRF24L01SendStr(char *str) {  
//  struct rec_spi_data *data; 
  
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
  uint8_t tmp_arr[3];
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
    uart_writeln("ok");
    return;
  } 
  if ((str[0] == 'R') && (str[1] == 'O') && (str[2] == 'M') && (str[3] == 'R')) {
    parse_HEX_string(str + 4, tmp_arr);
    uart_writelnHEX(EEPROM_read(tmp_arr[0] * 0xFF + tmp_arr[1]));
    return;
  } 
  _log(ERR_COM_PARSER_UNKNOW_COM);
}

void nRF24L01LoadConf(struct rec_nRF24L01_conf* rec_conf) {
  uint16_t addr;
  uint8_t *buff; 
  buff = (uint8_t *)rec_conf; 
  for(addr = NRF24L01_EEPROM; 
      addr < NRF24L01_EEPROM + sizeof(struct rec_nRF24L01_conf);
      addr++) {
    buff[addr - NRF24L01_EEPROM] = EEPROM_read(addr);
  }
}

void nRF24L01_init(void) {
  struct rec_spi_data *data;
  nRF24L01LoadConf(&nRF24L01_conf);
  
  nRF24L01SetRegister(0x20, nRF24L01_conf.config);
  nRF24L01SetRegister(0x21, nRF24L01_conf.en_aa);
  nRF24L01SetRegister(0x22, nRF24L01_conf.en_rxaddr);
  nRF24L01SetRegister(0x23, nRF24L01_conf.setup_aw);
  nRF24L01SetRegister(0x24, nRF24L01_conf.setup_retr);
  nRF24L01SetRegister(0x25, nRF24L01_conf.rf_ch);
  nRF24L01SetRegister(0x26, nRF24L01_conf.rf_setup);
  nRF24L01SetRegister(0x31, nRF24L01_conf.rx_pw_p0);
  nRF24L01SetRegister(0x32, nRF24L01_conf.rx_pw_p1);

  // Установка адреса для дефолтного канала приема 
  // RX_ADDR_P0 - Receive address data pipe 0. 5 Bytes maximum 
  // length. (LSByte is written first. Write the number of 
  // bytes defined by SETUP_AW).
  data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendBuff[0] = 0x2A;
  data->sendBuff[1] = nRF24L01_conf.rx_addr_p0[0];
  data->sendBuff[2] = nRF24L01_conf.rx_addr_p0[1];
  data->sendBuff[3] = nRF24L01_conf.rx_addr_p0[2];
  data->sendSize = 4;
  spiTransmit(0);
  
  data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendBuff[0] = 0x2B;
  data->sendBuff[1] = nRF24L01_conf.rx_addr_p1[0];
  data->sendBuff[2] = nRF24L01_conf.rx_addr_p1[1];
  data->sendBuff[3] = nRF24L01_conf.rx_addr_p1[2];
  data->sendSize = 4;
  spiTransmit(0);

  // Установка адреса для дефолтного канала передачи 
  // TX_ADDR - Transmit address. Used for a PTX device only. 
  // (LSByte is written first) Set RX_ADDR_P0 equal to this address 
  // to handle automatic acknowledge if this is a PTX device with 
  // Enhanced ShockBurst™ enabled  
  data = spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendBuff[0] = 0x30;
  data->sendBuff[1] = nRF24L01_conf.tx_addr[0];
  data->sendBuff[2] = nRF24L01_conf.tx_addr[1];
  data->sendBuff[3] = nRF24L01_conf.tx_addr[2];
  data->sendSize = 4;
  spiTransmit(0);
    
  // Включаем трансивер
  nRF24L01SetRegister(0x20, 0x02 | nRF24L01_conf.config);
  nRF24L01_UP;
}
/*
void timerTask(uint8_t *params) {
  nRF24L01SetRegister(0xE1, 0); // Очистить буфер передачи
  nRF24L01SetRegister(0x27, 0xFF); // Збросить прерывания
  nRF24L01SetRegister(0xA0, 0x30); // Отправить байт данных
  //nRF24L01SetRegister(0x20, 0x0A | nRF24L01_conf.config);
  nRF24L01SendActivate(0, 0);
}*/

int main(void) {
  uint8_t spi_read_buff[32];
  timer_init();
  uart_async_init();
  pcint_init(0);
  uart_readln(&commands_reciver);
  sei();
  uart_writeln("Start");
  spi_init(spi_read_buff);
  NRF24L01_SCN_DDR |= _BV(NRF24L01_SCN_DDN);
  NRF24L01_SCN_PORT |= _BV(NRF24L01_SCN_PIN);
  NRF24L01_CE_DDR |= _BV(NRF24L01_CE_DDN);
  NRF24L01_CE_PORT &= ~(_BV(NRF24L01_CE_PIN));
  nRF24L01_init();
  if (nRF24L01_conf.config & 1) {
    NRF24L01_CE_PORT |= _BV(NRF24L01_CE_PIN);
  } else {
    //timer1PutMainTask(&timerTask, 0);
  }
 
  NRF24L01_CE_PORT |= _BV(NRF24L01_CE_PIN);
  while(1) {
    sleep_mode();
  }
  return 0;
}
