#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "lsquaredc.h"

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

int I2CreadByte(uint16_t chipAddress, uint16_t registerAddress, uint8_t *byte_from_I2Cdevice, int I2Chandle)
{
/*
//	uint16_t i2c_event_sequence[] = {chipAddress<<1, registerAddress, I2C_RESTART, (chipAddress<<1)|1, I2C_READ, I2C_READ};
	uint16_t i2c_event_sequence[] = {chipAddress<<1, registerAddress, I2C_RESTART, (chipAddress<<1)|1, I2C_READ};	
	uint8_t data[2];   //data[0] is dummy.
	
//	int ioctlret = i2c_send_sequence(I2Chandle, i2c_event_sequence, 6, data);
	int ioctlret = i2c_send_sequence(I2Chandle, i2c_event_sequence, 5, data);
	if(ioctlret == 2)
	{
//		*byte_from_I2Cdevice = data[1];
		*byte_from_I2Cdevice = data[0];
		return 1;
	}
	else
	{
		return -1;
	}
*/
	uint16_t i2c_event_sequence[] = {chipAddress<<1, registerAddress, I2C_RESTART, (chipAddress<<1)|1, I2C_READ};	
	uint8_t data;
	int ioctlret = i2c_send_sequence(I2Chandle, i2c_event_sequence, 5, &data);
	printf("ioctlret=%d\n", ioctlret);
	if(ioctlret == 2)
	{
		*byte_from_I2Cdevice = data;
		return 1;
	}
	else
	{
		return -1;
	}
}


int I2CreadBlock(uint16_t chipAddress, uint16_t registerAddress, uint8_t *bytes_from_I2Cdevice, int blockLength, int I2Chandle)
{
	//uint16_t i2c_event_sequence[5+blockLength];
	uint16_t *i2c_event_sequence = (uint16_t *)malloc( sizeof(uint16_t)*(5+blockLength) );
	
	
	i2c_event_sequence[0] = chipAddress<<1;
	i2c_event_sequence[1] = registerAddress;
	i2c_event_sequence[2] = I2C_RESTART;
	i2c_event_sequence[3] = (chipAddress<<1)|1;
	i2c_event_sequence[4] = I2C_READ;           //dummy
	
	for(int i=5; i<5+blockLength; i++)
	{
		i2c_event_sequence[i] = I2C_READ;
	}
	
	uint8_t data_with_dummy[blockLength+1];
	int ioctlret = i2c_send_sequence(I2Chandle, i2c_event_sequence, 5+blockLength, data_with_dummy);
	
	
	free(i2c_event_sequence);
	if(ioctlret != 2)
	{
		
		return -1;
	}
	
	for(int i=1; i<=blockLength; i++)
	{
		bytes_from_I2Cdevice[i-1] = data_with_dummy[i];
	}
	
	
	return 1;
}

int I2CwriteByte(uint16_t chipAddress, uint16_t registerAddress, uint8_t byte_to_write, int handle)
{
	uint16_t i2c_event_sequence[] = {chipAddress<<1, registerAddress, 0xFF&byte_to_write};
	int ioctlret = i2c_send_sequence(handle, i2c_event_sequence, 3, &byte_to_write);
	
	if(ioctlret != 1)
	{
		return -1;
	}
	
	return 1;
}

int I2CwriteBlock(uint16_t chipAddress, uint16_t registerAddress, uint8_t *bytes_to_write, int blockLength, int handle)
{
	
	
	//uint16_t i2c_event_sequence[2+blockLength];
	uint16_t *i2c_event_sequence = (uint16_t *)malloc( sizeof(uint16_t)*(2+blockLength) );
	
	
	i2c_event_sequence[0] = chipAddress<<1;
	i2c_event_sequence[1] = registerAddress;
/*	
	i2c_event_sequence[2] = I2C_RESTART;
	i2c_event_sequence[3] = chipAddress<<1;
*/	
	for(int i=2; i<2+blockLength; i++)
	{
		i2c_event_sequence[i] = 0xFF&bytes_to_write[i-2];
	}
	
	//uint8_t dummy=7;
	int ioctlret = i2c_send_sequence(handle, i2c_event_sequence, 2+blockLength, bytes_to_write);
	
	free(i2c_event_sequence);
	//printf("ioctl_wb = %d\n", ioctlret);
	
	
/*
	int ioctlret;
	for(int i=0; i<blockLength; i++)
	{
		ioctlret = I2CwriteByte(chipAddress, registerAddress++, bytes_to_write[i], handle);
		if(ioctlret != 1)
		{
			return -1;
		}
	}
*/
	return 1;


}




int i2c_transfer(uint16_t chipAddress, uint16_t registerAddress, uint8_t *bytes_to_write, int blockLength, uint8_t *rx_buffer, int handle)
{
	//rx size is 2;
	
	uint16_t *i2c_event_sequence = (uint16_t *)malloc( sizeof(uint16_t)*(6+blockLength) );
	
	i2c_event_sequence[0] = chipAddress<<1;
	i2c_event_sequence[1] = registerAddress;
	
	for(int i=2; i<2+blockLength; i++)
	{
		i2c_event_sequence[i] = 0xFF&bytes_to_write[i-2];
	}
	
	i2c_event_sequence[2+blockLength] = I2C_RESTART;
	i2c_event_sequence[3+blockLength] = (chipAddress<<1)|1;
	//i2c_event_sequence[4+blockLength] = I2C_READ;           //dummy
	
	for(int i=4+blockLength; i<6+blockLength; i++)
	{
		i2c_event_sequence[i] = I2C_READ;
	}

	int ioctlret = i2c_send_sequence(handle, i2c_event_sequence, 6+blockLength, rx_buffer);
	//printf("ioctlret=%d\n", ioctlret);
	
	free(i2c_event_sequence);
	
	if(ioctlret != 2)
	{
		
		return -1;
	}
	
	return 1;

}

uint8_t get_checksum(uint8_t * start, int count)
{
	uint8_t sum = 0;
	int i;
	for(i=0; i<count; i++)
	{
		sum+=start[i];
	}
	sum++;
	return 0-sum;
}

int main() 
{
	int handle = i2c_open(1);
	printf("handle = %d\n", handle);

	
	
	unsigned long long int errors = 0;
	unsigned long long int transactions = 0;
	
	FILE * errcnt = fopen("errcnt", "w");
	if(!errcnt)
	{
		printf("cannot open file: errcnt\n");
		return -1;
	}
	printf("errcnt opened.\n");
	fseek(errcnt, 0, SEEK_SET);
	fprintf(errcnt, "errcnt = %llu\n", errors);
	fflush(errcnt);
	
	while(1)
	{
		/*
		int ioctlret = 0;		
		uint8_t rx;
		ioctlret = I2CreadByte(0x10, 0, &rx, handle);
		
		printf("rx = %d\n", rx);
		sleep(1);
		ioctlret = I2CwriteByte(0x10, 0, 8, handle);
		//printf("ioctlret = %d\n", ioctlret);
		sleep(1);
		*/
		
		uint8_t txb[]= {3, 0, 22};
		uint8_t rxb[2];
		int retry = 0;
		uint8_t c;
		transactions++;
		do
		{
		i2c_transfer(0x10, 0, txb, 3, rxb, handle); 
		printf("\t rx = [ %d  %d ]\n", rxb[0], rxb[1]);
		
		c = get_checksum(rxb, 1);
		
		if(c == rxb[1])
		{
			printf("\tchecksum good\n");
		}
		else
		{
			printf("\tchecksum not good. expected %d,  received %d   retry=%d\n", c, rxb[1], retry);
			fprintf(errcnt, "retry\n");
			fflush(errcnt);
			retry++;
		}
		}
		while(c!=rxb[1] && retry<10);
		if(c!=rxb[1])
		{
			//declare as error.
			errors++;
			printf("ERROR\n");
			fprintf(errcnt, "errcnt = %llu    transactions = %llu\n", errors, transactions);
			fflush(errcnt);
		}
		
		if(transactions%2000 == 0)
		{
			fprintf(errcnt, "errcnt = %llu    transactions = %llu\n", errors, transactions);
			fflush(errcnt);
		}
		
		//usleep(1);
		
		/*
		
		char block[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
		I2CwriteBlock(0x10, 0, block, 11, handle);
//		sleep(1);		
		for(volatile unsigned long long int d=0; d<=50000ULL; d++);
	
		uint8_t rx = 0;
		ioctlret = I2CreadByte(0x10, 9, &rx, handle);
		//printf("ioctlret=%d   rx=%d\n", ioctlret, rx);
		fflush(stdout);
//		sleep(1);
		for(volatile unsigned long long int d=0; d<=2000ULL; d++);
		
		transactions++;
		if(rx != 9)
		{
			errors++;			
			//fseek(errcnt, 0, SEEK_SET);
			fprintf(errcnt, "errcnt = %llu    transactions = %llu\n", errors, transactions);
			fflush(errcnt);
		}
		
		*/
		
		
		
		
		
		
		/*
		int i2cret;
		uint8_t rx=0;
		do
		{	
			printf("Checking readiness before sending adc command.\n");
			i2cret = I2CreadByte(0x10, _READY3, &rx, handle);
			printf("i2cret=%d   rx=%d\n", i2cret, rx);
			fflush(stdout);
			if(rx!=1){sleep(1);}
		}
		while( !(rx==1 && i2cret==1) );
		printf("Sending adc command.\n");
		i2cret = I2CwriteByte(0x10, _CMD3, 48, handle);
		usleep(25000);
		do
		{	
			usleep(1000);
			printf("Waiting before adc conversion.\n");
			i2cret = I2CreadByte(0x10, _READY3, &rx, handle);
			printf("i2cret=%d   rx=%d\n", i2cret, rx);
			fflush(stdout);
		}
		while( !(rx==1 && i2cret==1) );
		uint8_t replyH, replyL;
		i2cret = I2CreadByte(0x10, _REPLY3H, &replyH, handle);
		i2cret = I2CreadByte(0x10, _REPLY3L, &replyL, handle);
		printf("ADC = %d\n\n", (replyH<<8)|replyL);
		
		
		sleep(1);
		
		
		/*
		int i2cret;
		uint8_t ready2;
		do
		{
			i2cret = I2CreadByte(0x10, _READY2, &ready2, handle);
			printf("i2cret=%d   rx=%d\n", i2cret, ready2);
			fflush(stdout);
			if(ready2!=1)
			{
				usleep(1000);
			}
		}
		while(ready2!=1);
		i2cret = I2CwriteByte(0x10, _CMD2, 12, handle);
		sleep(1);
		
		do
		{
			i2cret = I2CreadByte(0x10, _READY2, &ready2, handle);
			printf("i2cret=%d   rx=%d\n", i2cret, ready2);
			fflush(stdout);
			if(ready2!=1)
			{
				usleep(1000);
			}
		}
		while(ready2!=1);
		i2cret = I2CwriteByte(0x10, _CMD2, 8, handle);
		sleep(1);
		*/
	}
	
	i2c_close(handle);
	return 0;
}
