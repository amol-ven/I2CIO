#include <avr/io.h>
#include <stdlib.h>
#define F_CPU 16000000UL
#include <util/delay.h>


int main()
{	
	DDRC = 0xFF;
	while(1)
	{
		//PORTC ^= 0xFF;
		_delay_ms(1000);
	}
}
