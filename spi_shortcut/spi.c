
#include <avr/io.h>
#include "UComMaster.h"
#include "spi.h"
#include <vt100.h>

void spi_init()
{
	// Setup SPI master for programming	
	DDRB = (1<<MOSI) | (1<<SCK);
    DDRB |= (1<<CS);
	DDRC |= (1<<4);
	DDRB &= ~(1<<MISO);
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPIE) | (1<<CPHA);
}

void select(void){ cbi(PORTB, CS); } //cbi(PORTC, 4); }

void reselect(void) { deselect(); delay_ms(1); select(); }

void deselect(void) { sbi(PORTB, CS); } //sbi(PORTC, 4); }

void send_spi_byte(char c)
{
	SPDR = c;
	while(!(SPSR & (1<<SPIF)));
}


char read_spi_byte(void)
{
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

