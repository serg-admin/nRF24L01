#!/usr/bin/perl
use Time::HiRes qw(usleep nanosleep); # Для задержки между блоками

if (@ARGV < 1 ) {
  print "Запуск команды:";
  print "    upload_conf.pl имя_файла";
}

$addr = 0x0000; # Адрес начала блока конфигурации
$cfg = $ARGV[0];

print "$cfg\n";
print `stty 38400 -F /dev/ttyUSB0`; # Устанавливаем параметры порта
open(PORT, ">/dev/ttyUSB0"); # Создаем поток в порт для записи
open DB, "<$cfg"; # Файл с конфигурацией

while(<DB>) {
  if (!(substr($_, 0, 1) eq '#')) {
    ($name, $data) = split "\t", $_;

    $value = "00";
    # Определяем в какой системе исчисления записаны данные,
    # и вычисляем HEX строку
    if (substr($data, 0, 1) eq 'b') { 
      $value = sprintf("%02X", oct $data); 
    }
    else {
      if (substr($data, 0, 2) eq '0x') {
        $value = substr($data, 2, length($data)-2);
      } else {
        if ($data < 256) {
          $value = sprintf("%02X", $data);
        }
        else {
          $value = sprintf("%04X", $data);
        }
      }
    }

    print "name:$name; data: $value\n";
    # Побайтная передача загружаемых данных в порт
    for($i = 0; $i < length($value); $i = $i + 2) {
      $result_str = sprintf("ROMW%04X" . substr($value, $i, 2), hex $addr);
      $addr++;
      print "command: $result_str\n";
      print PORT "$result_str\n";
      # Ждем пока отправится весь буфера COM порта.
      # Команда загрузки одного байта занимает ~12 байт:
      #   Прификс ROMW - 4 символа
      #   Адресс - 4 HEX символа
      #   Байт данных - 2 HEX символа
      #   Перевод строки \r\t - до двух байт
      # (время передачи примерно ~3200 микро секунд).
      # Для надежности и гарантии освобождения eeprom  ждем 6000 микро секунд.
      # 160 - байт в секунду (не очень оптимально вообщем-то, но пол секунды 
      # для загрузки конфикурации вполне комфорно).
      usleep 6000;
    }
  }
}
