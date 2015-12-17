#include <avr/io.h>
#include "tools/timer16.h"
#include "tools/uart_async.h"
#include "tools/error.h"
#include "tools/pcint.h"
#define PORT_SPI PORTB
#define DDR_SPI DDRB
#define DD_CE   DDB0
#define DD_IRQ  DDB1
#define DD_SCN  DDB2  // SS
#define PIN_SCN PORTB2
#define DD_MOSI DDB3
#define DD_MISO DDB4
#define DD_SCK  DDB5

#define ISP_SCRIPT_LEN 32
#define ISP_STATE_FREE 0
#define ISP_STATE_BUSY 1

struct rec_isp_script {
  uint8_t script[ISP_SCRIPT_LEN];
  uint8_t size;
  uint8_t pos;
  uint8_t status;
  uint8_t (* callBack)(uint8_t* buf, uint8_t size);
} isp_script;

uint8_t isp_transmit(void) {
  if (isp_script.status > 0) return isp_script.status;
  isp_script.pos = 0;
  SPDR = isp_script.script[isp_script.pos++];
  return 0;
}

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
          (pos < ISP_SCRIPT_LEN)) {
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
  PORT_SPI |= _BV(PIN_SCN);
  uart_writelnHEXEx(buf, size);
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
    isp_script.size = parse_HEX_string(str + 1, isp_script.script);
    if (! isp_script.size) {
      _log(ERR_COM_PARSER_PARS_ERR);
      return;
    }
    
    PORT_SPI &= ~(_BV(PIN_SCN));
    isp_script.callBack = &writelnHEX;
    isp_transmit();
  }
}

ISR (SPI_STC_vect) {
  isp_script.script[isp_script.pos - 1] = SPDR;
  if (isp_script.pos < isp_script.size) {
    SPDR = isp_script.script[isp_script.pos++];
  } else {
    isp_script.callBack(isp_script.script, isp_script.size);
  }
}

void SPI_MasterInit(void) {
  /* Set MOSI and SCK output, all others input */
  DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK) | _BV(DD_SCN) | _BV(DD_MISO);
  PORT_SPI |= _BV(PIN_SCN); // Переводим SPI Slave в сон.
  /* Enable SPI, Master, set clock rate fck/16 */
  SPCR = _BV(SPE)|(1<<MSTR)|(1<<SPR0) | _BV(SPIE); // Включаем прерывания
  isp_script.status = ISP_STATE_FREE;
}

int main(void) {
  timer_init();
  uart_async_init();
  SPI_MasterInit();
  pcint_init(0);
  uart_readln(&commands_reciver);
  sei();
  uart_writeln("Start");
  DDR_SPI |= _BV(DD_SCN);
  PORT_SPI |= _BV(PIN_SCN);
  while(1) {
    sleep_mode();
  }
  return 0;
}
