#ifndef __EEPROM_H_
#define __EEPROM_H_ 1
#include <avr/io.h>

/**
 * @brief Отправляет на запись в eeprom один байт 
 * @param uiAddress Адрес ячейки для записи
 * @param ucData Данные
 * @return 1- если eeprom занято, 0- если запись запущина успешно
 */
uint8_t EEPROM_write(unsigned int uiAddress, unsigned char ucData);

/**
 * @brief Блокирующие чтение с eeprom.
 * @param uiAddress
 * @return Возвращает прочитаные данные.
 */
uint8_t EEPROM_read(unsigned int uiAddress);

#endif