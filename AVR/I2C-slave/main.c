#include <avr/io.h>

#define F_CPU 16000000UL
#include <util/delay.h>
#include <util/twi.h>
#include <avr/interrupt.h>

#include <stdlib.h>

#include "registers.h"


#include "../amol-ven-Avark-757add0/SRC/avark.h"
typedef unsigned char byte_t;
typedef unsigned char bool_t;



//volatile uint8_t buffer_address;
//volatile uint8_t txbuffer[0xFF];
//volatile uint8_t rxbuffer[0xFF];


#define IDLE				0
#define MY_ADDR				1
#define PTR_RCVD			2

volatile int state = IDLE;
volatile int RegAddr;


volatile unsigned char Registers[REGISTERS];

K_STACK set1_stack[200];
K_STACK set2_stack[200];
K_STACK set3_stack[200];

/*       Event sequence of a IO command write to AVR from BBB.
 * 
 * The BBB writes to CMD
 * 
 * AVR ISR copies CMD to CMD_CP amd flips the ready bit
 * 
 * AVR tasks wait for CMD!=NOP and CMD_CP==NOP to execute cmd
 * 
 * Meanwhile BBB may poll CMD_CP to verify data health
 * 
 * If data is healthy BBB writes NOP to CMD_CP (which signals the AVR task to execute the command in CMD [which is same as CMD_CP])
 * BBB also checks for ready bit to flip (right not this is in case of IO commands). This tells BBB whether the AVR executed the command or not.
 * 
 * The AVR executes the command and flips the ready bit. AVR writes NOP back to CMD.
 * 
 * 
 *                  |<---ISR--->| 
 *    CMD     |  0  |  12 |  12 |  12 |  0 | 
 *    ready   |  1  |   1 |   0 |   0 |  1 |
 *    CMD_CP  |  0  |   0 |  12 |   0 |  0 |
 *                                    ^
 *                                    |--------Cmd is executed at this 'instance' 
 * 
 */ 


/*
//######## uart if needed here #########
void sendChar(volatile char a)
{
	while( !(UCSRA&(1<<UDRE)) );
	UDR=a;
}	


void sendString(volatile char *a)
{
	int index=0;

	while(a[(int)index]!='\0')
	{
		sendChar(a[(int)index]);
		index++;
	}
	index=0;
}

void sendULInt(unsigned long int x)
{
	char integer[15];
	char place=14;
	if(x==0)
	{
		sendChar('0');
	}
	while(x!=0)
	{
		integer[place]=(x%10)+48;
		x/=10;
		place--;
	}
	place++;
	while(place<=14)
	{
		sendChar(integer[place]);
		place++;
	}
}

//########################################
*/

void sendByte(byte_t b)
{
	char integer[3];
	int place=2;
	if(b==0)
	{
		sendChar('0');
	}
	while(b!=0)
	{
		integer[place]=(b%10)+48;
		b/=10;
		place--;
	}
	place++;
	while(place<=2)
	{
		sendChar(integer[place]);
		place++;
	}
}

void sendWord(unsigned int w)
{
	char integer[5];
	int place=4;
	if(w==0)
	{
		sendChar('0');
	}
	while(w!=0)
	{
		integer[place]=(w%10)+48;
		w/=10;
		place--;
	}
	place++;
	while(place<=4)
	{
		sendChar(integer[place]);
		place++;
	}
}

void set1()
{
	volatile int opr1, val1;
	sendString("set1");
	while(1)
	{
		//sendString("test1");
		opr1 = getOprFrmCmd(CMD1);
		if(opr1!=NOP && CMD1_CP==NOP)
		{
			sendString("cmd1");
			//READY1 = 0;
			val1 = getValFrmCmd(CMD1);
			switch(opr1)
			{
				case DRUG:
					if(val1)  {D1_PORT |= (1<<D1_PIN);}
					else      {D1_PORT &= ~(1<<D1_PIN);}
					break;
					
				case MEDIA:
					if(val1)  {M1_PORT |= (1<<M1_PIN);}
					else      {M1_PORT &= ~(1<<M1_PIN);}
					break;
					
				case AIR:
					if(val1)  {A1_PORT |= (1<<A1_PIN);}
					else      {A1_PORT &= ~(1<<A1_PIN);}
					break;

				case LIGHTS:
					if(val1)  {L1_PORT |= (1<<L1_PIN);}
					else      {L1_PORT &= ~(1<<L1_PIN);}
					break;
					
				case WASTE:
					if(val1)  {W1_PORT |= (1<<W1_PIN);}
					else      {W1_PORT &= ~(1<<W1_PIN);}
					break;
				
				case ADC_CMD:
					
					ADMUX = ( ADMUX&(~0x1F) )|SET1_ADC_CHANNEL;
					ADCSRA |= (1<<ADSC);
					
					while( !(ADCSRA&(1<<ADIF)) );
					
					byte_t h, l;
					l = ADCL;
					h = ADCH;
					unsigned int adcval = ((((unsigned int)h)<<8) | ((unsigned int)l));
					sendString("\tADCVAL1=");
					//sendULInt(adcval);
					sendWord(adcval);
					sendString("\n\r");
					//cli();while(1);
					
					REPLY1L = l;
					REPLY1H = h;
					break;
					
				default:
					break;		
			}
			CMD1 = NOP;
			CMD2_CP = NOP;
			sendString("READY1 = 1\n\r");
			
			READY1 = 1;
		}
		else
		{
			K_taskQuit();
		}
		//READY1 = 1;
	}
}

void set2()
{
	volatile int opr2, val2;
	sendString("set2");
	while(1)
	{
		opr2 = getOprFrmCmd(CMD2);
		if(opr2!=NOP && opr2==getOprFrmCmd(CMD2_CP))
		{
			//READY2 = 0;
			val2 = getValFrmCmd(CMD2);
			switch(opr2)
			{
				case DRUG:
					if(val2)  {D2_PORT |= (1<<D2_PIN);}
					else      {D2_PORT &= ~(1<<D2_PIN);}
					break;
					
				case MEDIA:
					if(val2)  {M2_PORT |= (1<<M2_PIN);}
					else      {M2_PORT &= ~(1<<M2_PIN);}
					break;
					
				case AIR:
					if(val2)  {A2_PORT |= (1<<A2_PIN);}
					else      {A2_PORT &= ~(1<<A2_PIN);}
					break;

				case LIGHTS:
					if(val2)  {L2_PORT |= (1<<L2_PIN);}
					else      {L2_PORT &= ~(1<<L2_PIN);}
					break;
					
				case WASTE:
					if(val2)  {W2_PORT |= (1<<W2_PIN);}
					else      {W2_PORT &= ~(1<<W2_PIN);}
					break;
				
				case ADC_CMD:
					//TODO
					//ADC MUTEX
					ADMUX = ( ADMUX&(~0x1F) )|SET2_ADC_CHANNEL;
					ADCSRA |= (1<<ADSC);
					
					while( !(ADCSRA&(1<<ADIF)) );
					
					byte_t h, l;
					l = ADCL;
					h = ADCH;
					unsigned int adcval = (((unsigned int)h)<<8) | (unsigned int)ADCL;
					sendString("\tADCVAL2=");
					//sendULInt(adcval);
					sendWord(adcval);
					sendString("\n\r");
					//cli();while(1);
					
					REPLY2L = l;
					REPLY2H = h;
					break;
					
				default:
					break;		
			}
			CMD2 = NOP;
			CMD2_CP = NOP;
			sendString("READY2 = 1\n\r");
			
			//READY2 = 1;
		}
		else
		{
			K_taskQuit();
		}
		READY2 = 1;
	}
}


void set3()
{
	volatile int opr3, val3;
	//DDRC |= 1<<7;
	//PORTC |= (1<<7);
	sendString("set3");
	while(1)
	{
		opr3 = getOprFrmCmd(CMD3);
		if(opr3 != NOP)
		{
			sendString("cmd3");
			//READY3 = 0;
			val3 = getValFrmCmd(CMD3);
			switch(opr3)
			{
				case DRUG:
					if(val3)  {D3_PORT |= (1<<D3_PIN);}
					else      {D3_PORT &= ~(1<<D3_PIN);}
					break;
					
				case MEDIA:
					if(val3)  {M3_PORT |= (1<<M3_PIN);}
					else      {M3_PORT &= ~(1<<M3_PIN);}
					break;
					
				case AIR:
					sendString("air3\n\r");
					if(val3)  {A3_PORT |= (1<<A3_PIN);}
					else      {A3_PORT &= ~(1<<A3_PIN);}
					break;

				case LIGHTS:
					if(val3)  {L3_PORT |= (1<<L3_PIN);}
					else      {L3_PORT &= ~(1<<L3_PIN);}
					break;
					
				case WASTE:
					if(val3)  {W3_PORT |= (1<<W3_PIN);}
					else      {W3_PORT &= ~(1<<W3_PIN);}
					break;
				
				case ADC_CMD:
					//TODO
					//ADC MUTEX
					ADMUX = ( ADMUX&(~0x1F) )|SET3_ADC_CHANNEL;
					ADCSRA |= (1<<ADSC);
					
					while( !(ADCSRA&(1<<ADIF)) );
					
					byte_t h, l;
					l = ADCL;
					h = ADCH;
					unsigned int adcval = (((unsigned int)h)<<8) | l;
					sendString("\tADCVAL3=");
					//sendULInt(adcval);
					sendWord(adcval);
					sendString("\n\r");
					//cli();while(1);
					
					REPLY3L = l;
					REPLY3H = h;
					break;
					
				default:
					sendString("DEF in set 3 switch\n\r");
					break;		
			}
			CMD3 = NOP;
			sendString("READY3 = 1\n\r");
			
			//READY3 = 1;
		}
		else
		{
			//sendString("nop3");
			K_taskQuit();
		}
		READY3 = 1;
	}
}


int main(void)
{
	DDRC = 0xFC;
	PORTC = 3;
	
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	//UBRRL = 103;      //9600
	UBRRL = 8;          //115200
	
	sendString("Start\n\r");
	
	
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	
	
	
	TWBR = 98;
	TWAR = 0x10<<1|1;
	//TWCR = (1<<TWEA)|(1<<TWEN)|(1<<TWIE);
	TWCR = (1<<TWIE) | (1<<TWEA) | (1<<TWINT) | (1<<TWEN);
	TWSR = 0;
	
	
	sei();
	
	
	
	READY1 = 1;
	READY2 = 1;
	READY3 = 1;
	
	
	//K_init(set1, &set1_stack[99], set2, &set2_stack[99]);
	K_init(set1, &set1_stack[199], set2, &set2_stack[199], set3, &set3_stack[99]);
	K_start();
	while(1);
	
	
	while(1)
	{
		/*
		PORTC |= (1<<7);
		_delay_ms(110);
		PORTC &= ~(1<<7);
		_delay_ms(110);
		
		PORTC |= (1<<7);
		_delay_ms(110);
		PORTC &= ~(1<<7);
		_delay_ms(800);
		*/
		
		
		
	}
	return 0;
}

//Decimal: 96 128 128 160
//Hex:     60  80  80  A0
ISR(TWI_vect)
{
	byte_t status = TWSR&0xF8;
	sendString("\t\tSTATUS=");
	sendByte(status);
	sendString("\n\r");
	byte_t data;
	//while(1);
	
	//sendChar('\t');
	//sendULInt(state);
	//sendString("\n\r");
	
	
	switch(status)
	{
		case TW_SR_SLA_ACK:			//0x60			96
			// SLA+W received. ACk returned.
			state = MY_ADDR;
			TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
			break;

		case TW_SR_DATA_ACK:		//0x80			128
			data = TWDR;
			if(state == MY_ADDR)
			{
				RegAddr = data;
				state = PTR_RCVD;
				sendString("Ptr set to ");
				sendByte(data);
				sendString("\n\r");
				TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
			}
			else if(state == PTR_RCVD)
			{
				// data now has valid data to be put at RegAddr
				// handle it.
				if(1)
				{
					//if you can receive more data. 
					//At the moment always true.
					//So send ACK back.
					/*
					sendString("\ni2cset ");
					sendULInt(data);
					sendString(" at ");
					sendULInt(RegAddr);
					sendString("\n\r");
					*/
					//PORTC = data&(~3);
					//if(RegAddr >= 0 && RegAddr < REGISTERS)
					if( (RegAddr>=_CMD1 && RegAddr<=_CMD3) || (RegAddr>=_CMD1_CP && RegAddr<=_CMD3_CP) )
					{
						Registers[RegAddr] = data;
						
						sendString("Set data as ");
						sendByte(data);
						sendString("\n\r");
						
						//sendString("RegAddr=");
						//sendWord(RegAddr);
						if(RegAddr == _CMD1)
						{
							READY1 = 0;
							CMD1_CP = CMD1;
						}
						else if(RegAddr == _CMD2)
						{
							READY2 = 0;
							CMD2_CP = CMD2;
						}
						else if(RegAddr == _CMD3)
						{
							READY3 = 0;
							CMD3_CP = CMD3;
						}
						
						RegAddr = (RegAddr+1)%REGISTERS;
					}
					else
					{
						//RegAddr out of bounds.
						sendString("RegAddr out of bounds.\n\r");
					}
					TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
				}
				else
				{
					//if you cannot receive more data. 
					//So send NACK back.
					TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEN); 
				}
			}
			break;
		
		case TW_SR_STOP:			//0xA0			160
			// stop condition or a repeated start condition has been received.
			state = IDLE;
			//TWCR |= (1<<TWEA) | (1<<TWINT) | (1<<TWEN);	
			TWCR |= (1<<TWIE) | (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
			break;
		
		
		
		
		case TW_ST_SLA_ACK:			//0xA8			168
			// SLA+R received. ACK returned. 	
			// Need to transmit data back to master.
			TWDR = Registers[RegAddr];
			RegAddr = (RegAddr+1)%REGISTERS;
			state = MY_ADDR;
			
			sendString("read data as ");
			sendByte(TWDR);
			sendString("\n\r");
			TWCR |= (1<<TWEA) | (1<<TWINT);
			break;
		
		case TW_ST_DATA_ACK:		//0xB8			184
			//Data transmitted back to master. ACK received. Transmit next data.
			TWDR = Registers[RegAddr];
			RegAddr = (RegAddr+1)%REGISTERS;
			state = MY_ADDR;
			
			sendString("read data as ");
			sendByte(TWDR);
			sendString("\n\r");
			TWCR |= (1<<TWEA) | (1<<TWINT);
			break;
		
		case TW_ST_DATA_NACK:		//0xC0			192
			state = IDLE;
			
			sendString("Close I2C\n\r");
			TWCR |= (1<<TWEA) | (1<<TWINT);
			break;
			
			
			
		case  TW_BUS_ERROR:
			//bus error. illegal start/stop condition.
			state = IDLE;
			TWCR |= (1<<TWSTO) | (1<<TWINT) | (1<<TWEA);
			break;
			
		default:
			// if none of the above apply prepare TWI to be addressed again
			state = IDLE;
			sendString("\n\rDEFAULT in ISR\n\r");
			sendString("\t\tSTATUS=");
			sendByte(status);
			sendString("\n\r");
			//TWCR |= (1<<TWIE) | (1<<TWEA) | (1<<TWEN);
			TWCR |= (1<<TWEA) | (1<<TWINT) | (1<<TWSTO);
			break;
	
	}

}

ISR(USART_RXC_vect)
{
	sendChar(UDR);
}

/*
//ISR(TWI_vect)
void ignore()
{
	
	// temporary stores the received data
	uint8_t data;
	sendULInt(TWSR&0xF8);while(1);
	// own address has been acknowledged
	if( (TWSR & 0xF8) == TW_SR_SLA_ACK ){  
		buffer_address = 0xFF;
		// clear TWI interrupt flag, prepare to receive next byte and acknowledge
		TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
	}
	else if( (TWSR & 0xF8) == TW_SR_DATA_ACK ){ // data has been received in slave receiver mode
		
		// save the received byte inside data 
		data = TWDR;
		
		// check wether an address has already been transmitted or not
		if(buffer_address == 0xFF){
			
			buffer_address = data; 
			
			// clear TWI interrupt flag, prepare to receive next byte and acknowledge
			TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
		}
		else{ // if a databyte has already been received
			
			// store the data at the current address
			rxbuffer[buffer_address] = data;
			
			// increment the buffer address
			buffer_address++;
			
			// if there is still enough space inside the buffer
			if(buffer_address < 0xFF){
				// clear TWI interrupt flag, prepare to receive next byte and acknowledge
				TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
			}
			else{
				// clear TWI interrupt flag, prepare to receive last byte and don't acknowledge
				TWCR |= (1<<TWIE) | (1<<TWINT) | (0<<TWEA) | (1<<TWEN); 
			}
		}
	}
	else if( (TWSR & 0xF8) == TW_ST_DATA_ACK ){ // device has been addressed to be a transmitter
		
		// copy data from TWDR to the temporary memory
		data = TWDR;
		
		// if no buffer read address has been sent yet
		if( buffer_address == 0xFF ){
			buffer_address = data;
		}
		
		// copy the specified buffer address into the TWDR register for transmission
		TWDR = txbuffer[buffer_address];
		// increment buffer read address
		buffer_address++;
		
		// if there is another buffer address that can be sent
		if(buffer_address < 0xFF){
			// clear TWI interrupt flag, prepare to send next byte and receive acknowledge
			TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
		}
		else{
			// clear TWI interrupt flag, prepare to send last byte and receive not acknowledge
			TWCR |= (1<<TWIE) | (1<<TWINT) | (0<<TWEA) | (1<<TWEN); 
		}
		
	}
	else{
		// if none of the above apply prepare TWI to be addressed again
		TWCR |= (1<<TWIE) | (1<<TWEA) | (1<<TWEN);
	} 
}
*/
