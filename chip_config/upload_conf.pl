#!/usr/bin/perl
use Time::HiRes qw(usleep nanosleep);

# Адрес начала блока конфигурации
$addr = 0x0000;

# Время последнего срабатывания всегда 0;
$last_alarm_time = '0000000000';
print `stty 38400 -F /dev/ttyUSB0`;
open(PORT, ">/dev/ttyUSB0");

$value = 0;
open DB, "<RF_CONFIG.csv";
while(<DB>) {
  if (!(substr($_, 0, 1) eq '#')) {
    ($name, $data) = split "\t", $_;

    $str = "";
    # Определяем в какой системе исчисления записаны параметры задачи
    if (substr($data, 0, 1) eq 'b') { $value = sprintf("%02X", hex(oct $data)); }
    else {
      if (substr($data, 0, 2) eq '0x') {
        $value = substr($data, 2, length($data)-3);
      } else {
        if ($data < 256) {
          $value = sprintf("%02X", $data);
        }
        else {
          $value = sprintf("%04X", $data);
        }
      }
    }

    $size = $size * 2;
    $str = $str .  $value;
    print "str = $str\n";
    for($i = 0; $i < length($str); $i = $i + 2) {
      $result_str = sprintf("ROMW%04X" . substr($str, $i, 2), hex $addr);
      $addr++;
      print "$result_str\n";
      print PORT "$result_str\n";
      # Ждем пока отправится весь буфера COM порта
      usleep 6000;
    }
  }
}
