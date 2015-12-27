#ifndef __NRF24L01_H_
#define __NRF24L01_H_
#include <avr/io.h>
#include <spi.h>
#if defined (__AVR_ATmega128__)
#  define NRF24L01_SCN_DDR DDRB
#  define NRF24L01_SCN_PORT PORTB
#  define NRF24L01_SCN_DDN DDB0
#  define NRF24L01_SCN_PIN PORTB0
#  define NRF24L01_CE_DDR DDRB
#  define NRF24L01_CE_PORT PORTB
#  define NRF24L01_CE_PIN PORTB5
#  define NRF24L01_CE_DDN DDB5
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

#define nRF24L01_UP NRF24L01_CE_PORT |= _BV(NRF24L01_CE_PIN)
#define nRF24L01_DOWN NRF24L01_CE_PORT &= ~(_BV(NRF24L01_CE_PIN))
#define nRF24L01_SET_SEND  nRF24L01SetRegister(0x20, nRF24L01_conf.config + 1)
#define nRF24L01_SET_RECIVE  nRF24L01SetRegister(0x20, nRF24L01_conf.config + 2)
#define NRF24L01_EEPROM 0x0000 //Адрес конфигурации ресивера в eeprom
#define NRF24L01_LEN_OF_DATA 18

uint8_t nRF24L01State[NRF24L01_LEN_OF_DATA + 2]; // Регистр состояния для обработки прерывания ресивера IRQ

struct rec_nRF24L01_data {
  uint8_t dataSize;
  uint8_t type;
  uint8_t data[16];
} nRF24L01Data;

struct rec_nRF24L01_conf{
  //uint8_t config; // регистр 0x00
  uint8_t en_aa; // регистр 0x01
  uint8_t en_rxaddr; // регистр 0x02
  uint8_t setup_aw; // регистр 0x03
  uint8_t setup_retr; // регистр 0x04
  uint8_t rf_ch; // регистр 0x05
  uint8_t rf_setup; // регистр 0x06
  uint8_t rx_addr_p0[3]; // регистр 0x0A Адрес принимающей стороны
  uint8_t rx_addr_p1[3]; // регистр 0x0B Наш адрес
  uint8_t tx_addr[3]; // регистр 0x10 Адрес принимающей стороны
  //uint8_t rx_pw_p0; // регистр 0x11
  //uint8_t rx_pw_p1; // регистр 0x12
} nRF24L01_conf;


void nRF24L01SetRegister(uint8_t reg, uint8_t b);

#endif