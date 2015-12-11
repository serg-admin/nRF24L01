#include "timer16.h"
//#include "error.h"

// Смещение таймера для переполнения через секунду
#define TCNT_MIN 3036

//Прерывания компараторов таймера
#define TIMER1_A_CHECK (TIMSK1 & _BV(OCIE1A))
#define TIMER1_A_EN TIMSK1 |= _BV(OCIE1A)
#define TIMER1_A_DIS TIMSK1 &= ~(_BV(OCIE1A))
#define TIMER1_B_CHECK (TIMSK1 & _BV(OCIE1B))
#define TIMER1_B_EN TIMSK1 |= _BV(OCIE1B)
#define TIMER1_B_DIS TIMSK1 &= ~(_BV(OCIE1B))

struct rec_timerTask {
  void (*func)(uint8_t* params);
  uint8_t* data;
} A, B;

/**
 * @brief Отложенное выполняет процедуры
 * @param delay Задержка - в единицах коутера TIMER1.
 * @param func  Сылка на звдачу.
 * @param data  Параметр передаваемый в отложенную задачу.
 */
void timer1PutTask(uint16_t delay, void (*func)(uint8_t*), uint8_t* data) {
  while (1) { // Буду спать здесь пока не активирую прерывание
    if (! TIMER1_A_CHECK) {
      A.func = func;
      A.data = data;
      delay = TCNT1 + delay;
      if (delay < timer16_start_value) delay += timer16_start_value;
      TIFR1 |= _BV(OCF1A);
      OCR1A = delay;
      TIMER1_A_EN;
      return;
    }
    if (! TIMER1_B_CHECK) {
      B.func = func;
      B.data = data;
      delay = TCNT1 + delay;
      if (delay < timer16_start_value) delay += timer16_start_value;
      OCR1B = delay;
      TIMER1_B_EN;
      return;
    }
    sleep_mode();
  }
}

ISR (TIMER1_COMPA_vect) {
  A.func(A.data);
  TIMER1_A_DIS;
}

ISR (TIMER1_COMPB_vect) {
  B.func(B.data);  
  TIMER1_B_DIS;
}

// Прерывание переполнения таймера
ISR (TIMER1_OVF_vect) {
  cli();
  ledSw;
  sei();
}

void timer_init() {
  timer16_start_value = TCNT_MIN;
  // Разрешить светодиод arduino pro mini.
  DDRD |= _BV(DDD5);
  // Делитель счетчика 256 (CS10=0, CS11=0, CS12=1).
  // 256 * 65536 = 16 777 216 (тактов)
  TCCR1B |= _BV(CS12);
  // Делитель счетчика 8 (CS10=0, CS11=1, CS12=0) - 60 кратное ускорение.
  //TCCR1B |= _BV(CS11);
  //TCCR1B |= _BV(CS10); //Включить для мигания  4 -ре секунды
  // Включить обработчик прерывания переполнения счетчика таймера.
  TIMSK1 = _BV(TOIE1);
  // PRR &= ~(_BV(PRTIM1));
  TCNT1 += timer16_start_value;
}