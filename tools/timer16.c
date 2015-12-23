#include "timer16.h"
//#include "error.h"

// Смещение таймера для переполнения через секунду
#define TCNT_MIN 3036

//Прерывания компараторов таймера
#if defined (__AVR_ATmega128__)
#  define TIMER1_A_CHECK (TIMSK & _BV(OCIE1A))
#  define TIMER1_A_EN TIMSK |= _BV(OCIE1A)
#  define TIMER1_A_DIS TIMSK &= ~(_BV(OCIE1A))
#  define TIMER1_B_CHECK (TIMSK & _BV(OCIE1B))
#  define TIMER1_B_EN TIMSK |= _BV(OCIE1B)
#  define TIMER1_B_DIS TIMSK &= ~(_BV(OCIE1B))
#else
#  define TIMER1_A_CHECK (TIMSK1 & _BV(OCIE1A))
#  define TIMER1_A_EN TIMSK1 |= _BV(OCIE1A)
#  define TIMER1_A_DIS TIMSK1 &= ~(_BV(OCIE1A))
#  define TIMER1_B_CHECK (TIMSK1 & _BV(OCIE1B))
#  define TIMER1_B_EN TIMSK1 |= _BV(OCIE1B)
#  define TIMER1_B_DIS TIMSK1 &= ~(_BV(OCIE1B))
#endif
struct rec_timerTask {
  void (*func)(uint8_t* params);
  uint8_t* data;
} A, B, M;

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
#if defined (__AVR_ATmega128__)
      TIFR |= _BV(OCF1A);
#else
      TIFR1 |= _BV(OCF1A);
#endif
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

void timer1PutMainTask(void (*func)(uint8_t*), uint8_t* data) {
      M.func = func;
      M.data = data;
}


ISR (TIMER1_COMPA_vect) {
  TIMER1_A_DIS;
  A.func(A.data);
}

ISR (TIMER1_COMPB_vect) {
  TIMER1_B_DIS;
  B.func(B.data);  
}

uint8_t p = 0;

// Прерывание переполнения таймера
ISR (TIMER1_OVF_vect) {
  cli();
  ledSw;
  sei();
  switch (p++) {
    case 1: if (M.func) M.func(M.data); break;
  }
  if (p > 5) p = 0;
}

void timer_init() {
  timer16_start_value = TCNT_MIN;
  // Разрешить светодиод arduino pro mini.
  DDRD |= _BV(DDD5);
  PORTD |= _BV(PORTD5);
  // Делитель счетчика 256 (CS10=0, CS11=0, CS12=1).
  // 256 * 65536 = 16 777 216 (тактов)
  TCCR1B |= _BV(CS12);
  // Делитель счетчика 8 (CS10=0, CS11=1, CS12=0) - 60 кратное ускорение.
  //TCCR1B |= _BV(CS11);
  //TCCR1B |= _BV(CS10); //Включить для мигания  4 -ре секунды
  // Включить обработчик прерывания переполнения счетчика таймера.
#if defined (__AVR_ATmega128__)
  TIMSK = _BV(TOIE1);
#else
  TIMSK1 = _BV(TOIE1);
#endif
  // PRR &= ~(_BV(PRTIM1));
  TCNT1 += timer16_start_value;
  M.func = 0;
}