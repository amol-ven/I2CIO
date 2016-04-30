#define REGISTERS		   15


#define _CMD1				0
#define _CMD2				1
#define _CMD3				2

#define _READY1				3
#define _READY2				4
#define _READY3				5

#define _REPLY1H			6
#define _REPLY1L			7

#define _REPLY2H			8
#define _REPLY2L			9

#define _REPLY3H		   10
#define _REPLY3L		   11

#define _CMD1_CP		   12
#define _CMD2_CP		   13
#define _CMD3_CP		   14


#define CMD1				Registers[_CMD1]
#define CMD2				Registers[_CMD2]
#define CMD3				Registers[_CMD3]

#define READY1				Registers[_READY1]
#define READY2				Registers[_READY2]
#define READY3				Registers[_READY3]

#define REPLY1H				Registers[_REPLY1H]
#define REPLY1L				Registers[_REPLY1L]

#define REPLY2H				Registers[_REPLY2H]
#define REPLY2L				Registers[_REPLY2L]

#define REPLY3H			    Registers[_REPLY3H]
#define REPLY3L			    Registers[_REPLY3L]

#define CMD1_CP				Registers[_CMD1_CP]
#define CMD2_CP				Registers[_CMD2_CP]
#define CMD3_CP				Registers[_CMD3_CP]



#define NOP			0
#define DRUG 		1
#define MEDIA		2
#define AIR			3
#define LIGHTS		4
#define WASTE		5
#define ADC_CMD		6


#define D1_DDR		DDRC
#define D1_PORT		PORTC
#define D1_PIN		2
#define M1_DDR		DDRC
#define M1_PORT		PORTC
#define M1_PIN		3
#define A1_DDR		DDRC
#define A1_PORT		PORTC
#define A1_PIN		4
#define L1_DDR		DDRC
#define L1_PORT		PORTC
#define L1_PIN		5
#define W1_DDR		DDRC
#define W1_PORT		PORTC
#define W1_PIN		6
#define SET1_ADC_CHANNEL	0



#define D2_DDR		DDRC
#define D2_PORT		PORTC
#define D2_PIN		7
#define M2_DDR		DDRD
#define M2_PORT		PORTD
#define M2_PIN		2
#define A2_DDR		DDRD
#define A2_PORT		PORTD
#define A2_PIN		3
#define L2_DDR		DDRD
#define L2_PORT		PORTD
#define L2_PIN		4
#define W2_DDR		DDRD
#define W2_PORT		PORTD
#define W2_PIN		5
#define SET2_ADC_CHANNEL	1


#define D3_DDR		DDRD
#define D3_PORT		PORTD
#define D3_PIN		6
#define M3_DDR		DDRD
#define M3_PORT		PORTD
#define M3_PIN		7
#define A3_DDR		DDRB
#define A3_PORT		PORTB
#define A3_PIN		0
#define L3_DDR		DDRB
#define L3_PORT		PORTB
#define L3_PIN		1
#define W3_DDR		DDRB
#define W3_PORT		PORTB
#define W3_PIN		2
#define SET3_ADC_CHANNEL	2
