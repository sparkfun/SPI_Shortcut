/*
 * twi.c
 *
 * Created: 8/16/2013 10:08:54 PM
 *  Author: nick
 */ 

#include <avr/io.h>

void twi_init()
{
	// Default to 400kHZ TWI Clock
	TWSR = (0<<TWPS1) | (0<<TWPS0);
	TWBR = (1<<TWBR7) | (1<<TWBR6) | (0<<TWBR5) | (0<<TWBR4) | (0<<TWBR3) | (0<<TWBR2) | (0<<TWBR1) | (0<<TWBR0);
	// Enable TWI
	TWCR = (1<<TWEN);
}

void twi_start(void)
{
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

void twi_stop(void)
{
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

void twi_write(uint8_t i)
{
	TWDR = i;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

uint8_t twi_readACK(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}

uint8_t twi_readNACK(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}

uint8_t twi_getstatus(void)
{
	uint8_t status;
	status = TWSR & 0xF8;
	return status;
}