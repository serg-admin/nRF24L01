#include "eeprom.h"
/**
 * @brief Отправляет на запись в eeprom один байт 
 * @param uiAddress Адрес ячейки для записи
 * @param ucData Данные
 * @return 1- если eeprom занято, 0- если запись запущина успешно
 */
uint8_t EEPROM_write(unsigned int uiAddress, unsigned char ucData) {
  /*  Wait for completion of previous write  */
  if (EECR & _BV(EEPE)) return 1;
  /* Set up address and Data Registers */
  EEAR = uiAddress;
  EEDR = ucData;
  /* Write logical one to EEMPE */
  EECR |= (1<<EEMPE);
  /* Start eeprom write by setting EEPE */
  EECR |= (1<<EEPE);
  return 0;
}

/**
 * @brief Блокирующие чтение с eeprom.
 * @param uiAddress
 * @return Возвращает прочитаные данные.
 */
uint8_t EEPROM_read(unsigned int uiAddress) {
  /* Wait for completion of previous write */
  while(EECR & (1<<EEPE));
  /* Set up address register */
  EEAR = uiAddress;
  /* Start eeprom read by writing EERE */
  EECR |= (1<<EERE);
  /* Return data from Data Register */
  return EEDR;
}