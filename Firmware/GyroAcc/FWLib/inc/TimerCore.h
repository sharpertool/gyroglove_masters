

#ifndef TIMERCORE_H
#define TIMERCORE_h

void timer_init();

unsigned long millis();
uint64_t micros_huge();
unsigned long micros();
void delay(unsigned long ms);
void delayMicroseconds(unsigned long ms);


#if defined(TCF1)
#define TIMERCORE_TIMER0 TCF0
#define TIMERCORE_TIMER1 TCF1
#define TIMERCORE_USE_TCF0
#define TIMERCORE_USE_TCF1

#elif defined(TCE1)
#define TIMERCORE_TIMER0 TCE0
#define TIMERCORE_TIMER1 TCE1
#define TIMERCORE_USE_TCE0
#define TIMERCORE_USE_TCE1
#endif

#endif
