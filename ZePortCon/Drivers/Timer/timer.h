/*
 * timer.h
 *
 * Created: 20.09.2017 20:14:11
 *  Author: Kirill
 */ 


#ifndef TIMER_H_
#define TIMER_H_



#ifndef F_CPU
	/* prevent compiler error by supplying a default */
	# warning "F_CPU not defined for <Drivers/Timer/timer.h>"
	# define F_CPU 1000000UL
#endif





#include <avr/io.h>
#include <avr/interrupt.h>
//#include "../../System/interrupts.h"


//
// прерывание таймера и подсчет прошедших секунд
//
// https://ucexperiment.wordpress.com/2012/03/16/examination-of-the-arduino-millis-function/
// https://github.com/arduino/Arduino/blob/2bfe164b9a5835e8cb6e194b928538a9093be333/hardware/arduino/avr/cores/arduino/wiring.c
//

/*
	Использование:

	TimerInit();

	uint64_t ms = millis();  // миллисекунд
	uint64_t s = ms/1000;	 // секунд

*/





#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )
 //#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
 //#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (F_CPU / 1000L) )


 #define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
 // the whole number of milliseconds per timer0 overflow
 #define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
 // the fractional number of milliseconds per timer0 overflow. we shift right
 // by three to fit these numbers into a byte. (for the clock speeds we care
 // about - 8 and 16 MHz - this doesn't lose precision.)
 #define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
 #define FRACT_MAX (1000 >> 3)



// включение таймера 0
void TimerInit();
// миллисекунды с момента запуска МК
unsigned long millis();
// микросекунды с момента запуска МК
unsigned long micros();


//ISR(TIMER0_OVF_vect);


#endif /* TIMER_H_ */