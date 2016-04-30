#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <util/twi.h>
#include <avr/interrupt.h>

#include "registers.h"

#define SERIAL


typedef unsigned char byte_t;
typedef unsigned char bool_t;



//volatile uint8_t buffer_address;
//volatile uint8_t txbuffer[0xFF];
//volatile uint8_t rxbuffer[0xFF];


#define IDLE            0
#define MY_ADDR            1
#define PTR_RCVD        2
#define PKT_RCVD        3
#define SENDING_RESP        4

volatile int state = IDLE;
volatile int inptr=0, outptr=0;


volatile unsigned char Incoming[50];
volatile unsigned char Outgoing[50];

void sendChar(volatile char a)
{
   while( !(UCSRA&(1<<UDRE)) );
   UDR=a;
}  


void sendString(volatile char *a)
{
#ifdef SERIAL
   int index=0;

   while(a[(int)index]!='\0')
   {
      sendChar(a[(int)index]);
      index++;
   }
   index=0;
#endif
}

void sendULInt(unsigned long int x)
{
#ifdef SERIAL
   char integer[15];
   int place=14;
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
#endif
}



void sendByte(byte_t b)
{
#ifdef SERIAL
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
#endif 
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

byte_t get_checksum(byte_t * start, int count)
{
   byte_t sum = 0;
   int i;
   for(i=0; i<count; i++)
   {
      sum+=start[i];
   }
   sum++;
   return 0-sum;
}

void prepare_response()
{
   byte_t len = Incoming[0];  //length of whole packet icluding len and data and checksum.
   int index_of_checksum = len-1;
   //data is from index 1 to index_of_checksum-1;
   //checksum is of len and data;
   byte_t expected_checksum = get_checksum(Incoming, len-1);
   if(expected_checksum == Incoming[index_of_checksum])
   {
      sendString("checksum good\n\r");
      Outgoing[0] = 1;
      Outgoing[1] = -2;
   }
   else
   {
      sendString("checksum bad   expected:");
      sendULInt(expected_checksum);
      sendString("received:");
      sendULInt(Incoming[index_of_checksum]);
      sendString("\n\r");
   }
   Outgoing[0] = 0xff;
   Outgoing[1] = 0xee;
}
   
   
   
int main()
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
   Outgoing[0] = 7;
   Outgoing[1] = 9;
   
   
   TCCR0 = 0;
   TCNT0 = 0;
   TIMSK = 1;
   
   
   sei();
   
   
   while(1);
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
      case TW_SR_SLA_ACK:        //0x60         96
         // SLA+W received. ACk returned.
         state = MY_ADDR;
         TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
         break;

      case TW_SR_DATA_ACK:    //0x80         128
         data = TWDR;
         if(state == MY_ADDR)
         {
            inptr = data;
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
         
               if(inptr>=0 && inptr<(sizeof(Incoming)/sizeof(Incoming[0])))
               {
                  Incoming[inptr] = data;
                  
                  sendString("Set data as ");
                  sendByte(data);
                  sendString("\n\r");
                  
                                    
                  inptr = (inptr+1)%(sizeof(Incoming)/sizeof(Incoming[0]));
               }
               else
               {
                  //inptr out of bounds.
                  sendString("Inptr out of bounds.\n\r");
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
      
      case TW_SR_STOP:        //0xA0         160
         // stop condition or a repeated start condition has been received.
         sendString("stop or repeated start.\n\r");
         if(state == PTR_RCVD)
         {
            state = PKT_RCVD;
            sendString("state is pkt rcvd.\n\r");
            //prepare a response
            prepare_response();
            outptr = 0;
         }
         else
         {
            state = IDLE;
            sendString("state is idle.\n\r");
            //TWCR |= (1<<TWEA) | (1<<TWEN); 
            //TWCR |= (1<<TWEA) | (1<<TWINT) | (1<<TWEN);   
            TWCR |= (1<<TWIE) | (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
            
         }     
      
         break;
      
      case TW_ST_SLA_ACK:        //0xA8         168
         // SLA+R received. ACK returned.    
         // Need to transmit data back to master.
         TWDR = Outgoing[outptr];
         outptr = (outptr+1)%(sizeof(Outgoing)/sizeof(Outgoing[0]));
         state = SENDING_RESP;
         
         sendString("read data as ");
         sendByte(TWDR);
         sendString("\n\r");
         TWCR |= (1<<TWEA) | (1<<TWINT);
         break;
      
      case TW_ST_DATA_ACK:    //0xB8         184
         //Data transmitted back to master. ACK received. Transmit next data.
         TWDR = Outgoing[outptr];
         outptr = (outptr+1)%(sizeof(Outgoing)/sizeof(Outgoing[0]));
         state = SENDING_RESP;
         
         sendString("read data as ");
         sendByte(TWDR);
         sendString("\n\r");
         TWCR |= (1<<TWEA) | (1<<TWINT);
         break;
      
      case TW_ST_DATA_NACK:      //0xC0         192
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
