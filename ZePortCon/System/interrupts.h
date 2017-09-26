/*
 * interrupts.h
 *
 * Created: 20.09.2017 20:07:32
 *  Author: Kirill
 */ 



/*

	��������� ���������� � ��

*/


#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_



#include <avr/io.h>
//#include <avr/interrupt.h>




//
// �������� ���������� �� INT1
//
/*
	ISR(INT1_vect)
	{
		// do some work..
		MCUCR = (1<<ISC11) | (1<<ISC10); //! ������� ��������� �����, ��� �������, ����� �� ��������� ��� �� ����� !!!!
		//reti();
	}
*/
inline void sysInterruptInitInt1() 
{
	MCUCR = (1<<ISC11) | (1<<ISC10);
	GICR = (1<<INT1);
}



//
// �������� ������ �� TIMER0
//
/*
	ISR(TIMER0_OVF_vect)
	{
		// do some work..
	}
*/
void sysInterruptInitTimer0(bool withSei)
{
	TIMSK=(1<<TOIE0) | (1<<TOIE1);
	// set timer0 counter initial value to 0
	TCNT0=0x00;
	// start timer0 with /1024 prescaler
	TCCR0 = (1<<CS02) | (1<<CS00);
	// Enable global interrupts
	if (withSei) sei();
}





#endif /* INTERRUPTS_H_ */