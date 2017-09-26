/*
 * timer.cpp
 *
 * Created: 20.09.2017 20:14:03
 *  Author: Kirill
 */ 



#include "timer.h"

#include "../../System/interrupts.h"



//unsigned long timer0_millis = 0;
//unsigned char timer0_fract = 0;
//int timer0_overflow_count = 0;
volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;



void TimerInit()
{
	//TIMSK=(1<<TOIE0) | (1<<TOIE1);
	//// set timer0 counter initial value to 0
	//TCNT0=0x00;
	//// start timer0 with /1024 prescaler
	//TCCR0 = (1<<CS02) | (1<<CS00);
	//// Enable global interrupts
	//sei();
	
	sysInterruptInitTimer0(false);
}
ISR(TIMER0_OVF_vect)
{
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;
	
	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}
	
	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

// safe access to millis counter
unsigned long millis()
{
	unsigned long m;
	uint8_t oldSREG = SREG; // запоминаем состояние флагов прерывания
	cli();	// выключаем прерывания глобально
	m = timer0_millis;
	SREG = oldSREG;	// восстанавливаем прерывания
	return m;
}
unsigned long micros()
{
	//unsigned long m;
	unsigned long m;
	uint8_t oldSREG = SREG, t;
	
	cli();
	m = timer0_overflow_count;
	#if defined(TCNT0)
		t = TCNT0;
	#elif defined(TCNT0L)
		t = TCNT0L;
	#else
		#error TIMER 0 not defined
	#endif

	#ifdef TIFR0
		if ((TIFR0 & _BV(TOV0)) && (t < 255))
		m++;
	#else
		if ((TIFR & _BV(TOV0)) && (t < 255))
		m++;
	#endif

	SREG = oldSREG;
	
	return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}