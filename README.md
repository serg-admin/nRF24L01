ЧТО ЭТО
--------
Тестирование и работа ресивера на базе nRF24L01, написана C.
Сборка avr-gcc или Atmel studio.

Программа реализует мини чат между двумя устройствами. Передача
данных в ресивер осуществляется через UART (скорость порта 38400 бод)

Программа протестирована для MCU atmega128, atmega328. 

ПОДКЛЮЧЕНИЕ nRF24L01 к MCU
--------------------------

для Atmega328:

MCU	nRF24L01	arduino pro mini (nano)
-----------------------------------------------
MISO	MISO		pin-12
MOSI	MOSI		pin-11
SCK	SCK		pin-13
SS	SCN		pin-10
PORTB1	CE		pin-13
PORTB0	IRQ		pin-12

Для Atmega128:

MCU	nRF24L01
----------------
MISO	MISO
MOSI	MOSI
SCK	SCK
PORTB0	SCN
PORTB5	CE
PORTD0	IRQ

Для Atmega8515:

MCU	nRF24L01
----------------
MISO	MISO
MOSI	MOSI
SCK	SCK
PORTB2	SCN
PORTB1	CE
PORTD2	IRQ


ИНСТАЛЯЦИЯ, НАСТРОЙКА
---------------------
- Отредактируйте Makefile для компиляции по нужный контроллер - параметр mcu
    #mcu          = atmega328p
    mcu          = atmega128
- Выполните команду make
- Загрузити полученный файл main.hex в контроллер
   Можно использовать специальные цели make
   make pro_mini - загрузка в Arduino pro mini (необходимо в Makefile установить номер порта):
      pro_mini: $(programm).hex
        avrdude -c arduino -F -P /dev/ttyUSB0 -p $(mcu) -b 57600  -U flash:w:$(programm).hex:i
                                 ------------
   или
   make lc_studio - для загрузки в Atmega128 через ISP программатора STK500
   make atmega8515 - для загрузки в Atmega8515 через ISP программатора STK500
- Перейдите в каталог chip_config
- Откройте и отредактируйте файл upload_conf.pl

    print `stty 38400 -F /dev/ttyUSB2`; # Устанавливаем параметры порта
    open(PORT, ">/dev/ttyUSB2"); # Создаем поток в порт для записи

  Установите имена порта в соответсвии с конфигурацией вашей системы

- Загрузити конфигурацию сети в два MCU командами
   ./upload_conf.pl RECIVER_CONF.csv
   ./upload_conf.pl SENDER_CONF.csv
- Перезагрузите MCU с подключенным nRF24L01,  для применения конфигурации.

УПРАВЛЕНИЕ ЧЕРЕЗ FTDI/USART
---------------------------
Подключитесь к порту USART контроллера - скорость 34800 бод.
Через терминал в программу можно отправлять команды:

SENDHello	- Отправит строку Hello на второй контроллер.
SPIXXXXXXXX	- Отправляет команду на рессивер через SPI шину - 
                  выведет полученные в ответ данные XX - HEX строка
ROMRXXXX	- Прочитать и вывести байт из eeprom - адрес XXXX - два байта адреса HEX
ROMWXXXXXX	- Записать байт в eeprom - XXXX - адресс XX - байт данных
INB		- Поспотреть состояние порта "B" контроллера
OUTBXX		- Вывести байт на ноги порта "B" контроллера

СТРУКТУРА ПЕРЕДАВАЕМЫХ ДАННЫХ
-----------------------------
Программа и ресиверы настраиваются для использования пакетов постоянной длины.
Пакет представляет собой структуру из трех полей

size 1 байт - размер занчащих данных в пакете
type 1 байт - тип/функция данных пакета:
                0- вывести строку в порт USART
                1- вывести байты ввиде HEX строки в USART
data 16 байт - данные.

