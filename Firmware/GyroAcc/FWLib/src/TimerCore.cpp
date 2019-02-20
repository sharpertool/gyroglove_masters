/*
  TimerCore.c - Timer portion of wiring.c, extracted from
  Arduino code base, and adapted to use my class based timers.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

*/


#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#if defined(TCF1)
    #define TIMERA TCF0
    #define TIMERB TCF1
#elif defined(TCE1)
    #define TIMERA TCE0
    #define TIMERB TCE1
#endif

void timer_init()
{
    /* Timer 1 uses the system clock as its source. 32Mhz with a prescalar of 8 is 4Mhz.
     * 4 ticks per microsecond. 4000 ticks per milli. On overflow generate an event.
     *
     * Timer 2 uses timer 1's overlow event as its source. Each tick is a ms.
     * 1000 ticks per second. On overflow generates an interrupt.
     *
     * interrupt handler increments seconds counter.
    */
    /* This is the code from wiring.c
     * The key things that need to happen are to setup a timer
     * with a 1ms rate and increment the millis count.
     * In the Arduino they use a second timer that is triggered
     * by the event system and the first timer. I'm thinking that
     * this is more efficient than just having the first timer
     * increment the second whenever it counts 1000 times? This would
     * require that we do a few more machine instrcutions PER 1ms timer
     * tick.. but this could be kept to a minimum could it not?
     */
    TIMERA.CTRLA    = TC_CLKSEL_DIV8_gc;
    TIMERA.PERBUF   = 4000;
    TIMERA.CTRLB    = ( TIMERA.CTRLB & ~TC0_WGMODE_gm ) | TC_WGMODE_NORMAL_gc;
    //      EVSYS.CH0MUX  = EVSYS_CHMUX_TIMERA_OVF_gc;
    TIMERA.INTCTRLA = TC_OVFINTLVL_HI_gc;
    
    TIMERB.CTRLA    = TC_CLKSEL_EVCH0_gc;
    TIMERB.PERBUF   = 1000;
    TIMERB.CTRLB    = ( TIMERB.CTRLB & ~TC1_WGMODE_gm ) | TC_WGMODE_NORMAL_gc;
    TIMERB.CTRLD    = TC_EVACT_UPDOWN_gc | TC1_EVDLY_bm;
    TIMERB.INTCTRLA = TC_OVFINTLVL_HI_gc;
}

volatile unsigned long millis_count = 0;
volatile unsigned long seconds_count = 0;

#if defined(TCF1)
ISR(TCF0_OVF_vect)
{
    ++millis_count;
    EVSYS.STROBE = 0xF;
}

ISR(TCF1_OVF_vect)
{
    ++seconds_count;
}
#elif defined(TCE1)
ISR(TCE0_OVF_vect)
{
    ++millis_count;
    EVSYS.STROBE = 0xF;
}

ISR(TCE1_OVF_vect)
{
    ++seconds_count;
}
#endif

unsigned long millis(void)
{
    // disable interrupts while we read millis_count or we might get an
    // inconsistent value (e.g. in the middle of a write to millis_count)

    uint8_t oldSREG = SREG; // Save and restore the interrupt enable bit
    cli();
    unsigned long result = seconds_count*1000UL + TIMERB.CNT;
    SREG = oldSREG;

    return result + millis_count;
}

uint64_t micros_huge(void) {
    // disable interrupts while we read rtc_millis or we might get an
    // inconsistent value (e.g. in the middle of a write to rtc_millis)

        // TODO: Will micros and millis be consistent?
        // Are events processed even when interrupts are disabled?
        // If so TIMERB count may be updated even when interrupts are off.
    uint8_t oldSREG = SREG; // Save and restore the interrupt enable bit
    cli();
    uint64_t result = ((uint64_t)seconds_count)*1000000UL + TIMERB.CNT*1000UL + (TIMERA.CNT>>2);
    SREG = oldSREG;

    return result;
}

unsigned long micros(void) {
    // disable interrupts while we read rtc_millis or we might get an
    // inconsistent value (e.g. in the middle of a write to rtc_millis)

        // TODO: Will micros and millis be consistent?
        // Are events processed even when interrupts are disabled?
        // If so TIMERB count may be updated even when interrupts are off.
    uint8_t oldSREG = SREG; // Save and restore the interrupt enable bit
    cli();
    unsigned long result = seconds_count*1000000UL + millis_count*1000 + TIMERB.CNT*1000UL + (TIMERA.CNT>>2);
    SREG = oldSREG;

    return result;
}

void delay(unsigned long ms)
{
    unsigned long start = millis();
    
    while (millis() - start <= ms);
}

/* Delay for the given number of microseconds.  Assumes a 8 or 16 MHz clock. */
void delayMicroseconds(unsigned long us)
{
    unsigned long start = micros();
    
    while (micros() - start <= us);
}

