#include "eeprom.h"
/**
 * @brief Отправляет на запись в eeprom один байт 
 * @param uiAddress Адрес ячейки для записи
 * @param ucData Данные
 * @return 1- если eeprom занято, 0- если запись запущина успешно
 */
uint8_t EEPROM_write(unsigned int uiAddress, unsigned char ucData) {
  /*  Wait for completion of previous write  */
#if defined (__AVR_ATmega128__)
  if (EECR & _BV(EEWE)) return 1;
#elif defined (__AVR_ATmega8515__)
  if (EECR & _BV(EEWE)) return 1;
#else
  if (EECR & _BV(EEPE)) return 1;
#endif
  /* Set up address and Data Registers */
  EEAR = uiAddress;
  EEDR = ucData;
  /* Write logical one to EEMPE */
#if defined (__AVR_ATmega128__)
  EECR |= _BV(EEMWE);
  /* Start eeprom write by setting EEPE */
  EECR |= _BV(EEWE);
#elif defined (__AVR_ATmega8515__)
  EECR |= _BV(EEMWE);
  /* Start eeprom write by setting EEPE */
  EECR |= _BV(EEWE);
#else
  EECR |= _BV(EEMPE);
  /* Start eeprom write by setting EEPE */
  EECR |= _BV(EEPE);
#endif  
  return 0;
}

/**
 * @brief Блокирующие чтение с eeprom.
 * @param uiAddress
 * @return Возвращает прочитаные данные.
 */
uint8_t EEPROM_read(unsigned int uiAddress) {
  /* Wait for completion of previous write */
#if defined (__AVR_ATmega128__)
  if (EECR & _BV(EEWE)) return 1;
#elif defined (__AVR_ATmega8515__)
  if (EECR & _BV(EEWE)) return 1;
#else
  if (EECR & _BV(EEPE)) return 1;
#endif
  /* Set up address register */
  EEAR = uiAddress;
  /* Start eeprom read by writing EERE */
  EECR |= (1<<EERE);
  /* Return data from Data Register */
  return EEDR;
}