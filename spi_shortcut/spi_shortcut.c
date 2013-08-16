/*
 * spi_shortcut.c
 *
 * Created: 6/9/2013 2:48:16 AM
 *  Author: 
 */ 


#include "ft245.h"
#include <avr/io.h>
#include "UComMaster.h"
#include "spi.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/crc16.h>
// #include "vt100.h"

volatile uint8_t current_phase;
volatile uint8_t current_dorder;
volatile uint8_t current_frequency;

volatile uint16_t bits;
volatile uint8_t cnt;
volatile uint16_t fcnt=0;

uint8_t sendonly = 1;
volatile uint16_t send_string[256];

/* vt100 support?
u08 line;
u08 col; */


char getc245_blocking(void);
void send_ascii(void);
void toggle_mode(void);
void menu(void);
void spi_menu(void);
void twi_menu(void);
void settings_menu(void);
void send_single_ascii(void);
void send_single_hex(void);
void send_file(void);
void continuous_receive(void);
void single_receive(void);
void send_hex_string(void);
void send_ascii_string(void);
void send_command_string(void);
void set_polarity_phase(void);
void my_printf(uint8_t string_num);
void change_frequency(void);
void set_dorder(void);
void show_settings(void);
void init_settings(void);
void save_settings(void);

const char main_menu[] PROGMEM = "\n\r\n\r------SparkFun SPI Shortcut------\n\r\n\rMAIN MENU:\n\r(1) SPI Actions\n\r(2) TWI Actions\n\r(3) Settings\n\r\n\r";	
const char arrow[] PROGMEM = "->";
const char invalid[] PROGMEM = "Invalid Character\n\r";
const char send_single[] PROGMEM = "Enter characters to send, press enter to return to menu\n\r->";
const char action_menu[] PROGMEM = "\n\rACTIONS MENU:\n\r(1) Send command string\n\r(2) Send ASCII characters\n\r(3) Continuous receive\n\r(4) Return to main menu\n\r\n\r";
const char cont_receive[] PROGMEM = "Receiving, CTRL+C to stop\n\r";
const char send_command[] PROGMEM = "Enter hex string of 256 values or less. Press return when finished.\n\rRR = Receive, CH = Chip Select High, CL = Chip Select Low, DY = 10ms Delay\n\r";
const char cs_high [] PROGMEM = "CS High\n\r";
const char cs_low[] PROGMEM = "CS Low\n\r";
const char string_sent[] PROGMEM = "String sent!\n\r";
const char settings[] PROGMEM = "\n\rSETTINGS MENU:\n\r(1) Set clock polarity and phase\n\r(2) Set frequency\n\r(3) Set data order\n\r(4) Show current settings\n\r(5) Save Settings to EEPROM\n\r (6) Return to main menu\n\r";
const char set_polarity_1[] PROGMEM = "\n\rClock settings can be defined in the following ways:\n\r\n\rCPOL/CPHA___LEADING EDGE________TRAILING EDGE________MODE\n\r0/0         Sample (Rising)     Setup (Falling)       (1)\n\r";
const char set_polarity_2[] PROGMEM = "0/1         Setup (Rising)      Sample (Falling)      (2)\n\r1/0         Sample (Falling)    Setup (Rising)        (3)\n\r1/1         Setup (Falling)     Sample(Rising)        (4)\n\r\n\r";
const char set_polarity_3[] PROGMEM = "Change to mode:";
const char mode_changed[] PROGMEM = "\n\rMode changed!\n\r\n\r";
const char frequency_menu_1[] PROGMEM = "\n\rFrequency Options:\n\r\n\rMODE    EFFECTIVE FREQUENCY\n\r(1)          4MHz\n\r(2)          2MHz\n\r(3)          1MHz\n\r(4)          500kHz\n\r(5)          250kHz\n\r";
const char frequency_menu_2[] PROGMEM = "(6)          125kHz\n\r(7)          62.5kHz\n\r\n\rNew frequency mode: ";
const char frequency_changed[] PROGMEM = "Frequency changed!\n\r\n\r";
const char dorder_menu[] PROGMEM = "\n\rData Order Modes:\n\r\n\r(0)   MSB transmitted first\n\r(1)   LSB transmitted first\n\rNew data mode: ";
const char dorder_changed[] PROGMEM = "Data order changed!\n\r\n\r";
const char cur_settings[] PROGMEM = "\n\rCurrent Settings:\n\r";
const char data_order[] PROGMEM = "Data Order: ";
const char msb[] PROGMEM = "MSB\n\r\n\r";
const char lsb[] PROGMEM = "LSB\n\r\n\r";
const char command_delay[] PROGMEM = "Delay 10ms\n\r";
const char settings_corrupt[] PROGMEM = "\n\rSettings corrupted in EEPROM, resetting.\n\r";

PGM_P const string_table[] PROGMEM = 
{
	main_menu,
	arrow,
	invalid,
	send_single,
	action_menu,
	cont_receive,
	send_command,
	cs_high,
	cs_low,
	string_sent,
	settings,
	set_polarity_1,
	set_polarity_2,
	set_polarity_3,
	mode_changed,
	frequency_menu_1,
	frequency_menu_2,
	frequency_changed,
	dorder_menu,
	dorder_changed,
	cur_settings,
	data_order,
	msb,
	lsb,
	command_delay,
	settings_corrupt
};


char buffer[200];

int main(void)
{
	DDRB |= (1<<LED);
	DDRC |= ((1<<SDA) | (1<<SCL));
	PORTB |= (1<<LED);
	for(int i = 0; i < 3; i++) // Test sequence
	{
		PORTC |= (1<<SDA);
		delay_ms(5);
		PORTC &= ~(1<<SDA);
		PORTC |= (1<<SCL);
		delay_ms(5);
		PORTC &= ~(1<<SCL);
	}
	PORTC |= ((1<<SDA) | (1<<SCL));
	ioinit();
	init_settings();
	spi_init(); 
	menu();
	

}

void menu()
{
	char c = 0; 
	deselect();	

	
	my_printf(MAIN_MENU); // Print menu
	my_printf(ARROW);
	
	c = getc245_blocking();
	printf245("%c\n\r",c);

	
	switch(c)
	{
		case '1':
			spi_menu();
			break;
		case '2':
			twi_menu();
			break;
		case '3':
			settings_menu();
			break;
		default:
		{
			my_printf(INVALID);
			menu();
			break;
		}
	}
}

/*************************************************************************** 
	ACTIONS SECTION:       
		Actions include
			-Send command string
			-Send ASCII one at a time
			-Continuous Receive 
****************************************************************************/
void spi_menu(void)
{
	char c = 0;
	
	my_printf(ACTIONS_MENU);
	my_printf(ARROW);
	c = getc245_blocking();
	printf245("%c\n\r\n\r",c);
	switch(c)
	{
		case '1':
			send_command_string();
			break;
		case '2':
			send_single_ascii();
			break;
		case '3':
			continuous_receive();
			break;
		case '4':
			menu();
			break;
		default:
		{
			my_printf(INVALID);
			spi_menu();
			break;
		}
	}
}

void send_single_ascii(void)
{
	char c = 0;
	my_printf(SEND_SINGLE);
	while(1)  
	{
		c = getc245_blocking();
		if(c == 13){ printf245("\n\r\n\r"); spi_menu(); }
		printf245("Sent %c\n\r",c);
		select();
		send_spi_byte(c);
		deselect();
	}
}



void continuous_receive(void)
{
	char c = 0;
	uint8_t i = 0;
	my_printf(CONT_RECEIVE);
	while(c != 3)
	{
		send_spi_byte(0x00);
		printf245("%x ",SPDR);
		i++;
		c = getchar245();
		if(i == 15){ printf245("\n\r"); i = 0; }
	}
	printf245("\n\r\n\r");
	spi_menu();
}

void send_command_string(void)
{
	char h = 0, l = 0;
	uint16_t i = 0, j = 0, receive = 1;
	my_printf(SEND_COMMAND);
	while(i < 256)
	{
		h = getc245_blocking();
		if(h == 13){ break; } 
		printf245("%c",h);
		l = getc245_blocking();
		if(l == 13){ break; }
		printf245("%c ",l);
		
		if(h == 'R') // Receive
		{
			send_string[i] = 0x0100;
		}
		else if(l == 'H') // CS High
		{
			send_string[i] = 0x0101;
		}
		
		else if(l == 'L') // CS Low
		{
			send_string[i] = 0x0102;
		}
		else if(l == 'Y')
		{
			send_string[i] = 0x0103;
		}
		else
		{
			if(h >= 48 && h <= 57){ h -= 48; } 	  // h is a number
			else if(h >= 65 && h <= 70){ h -= 55; } // h is a letter
			else if(h >= 97 && h <= 102){ h -= 87; } // h is a letter
			else{ my_printf(INVALID); send_command_string(); }
		
			if(l >= 48 && l <= 57){ l -= 48; } 	  // l is a number
			else if(l >= 65 && l <= 70){ l -= 55; } // l is a letter
			else if(l >= 97 && l <= 102){ l -= 87; } // l is a letter
			else{ my_printf(INVALID); send_command_string(); }
		
			h = (h<<4) + l; 
			send_string[i] = h;
		}
		
		i++;
			
	}
	printf245("\n\r");
	if(i == 256){ i--; }
	for(j = 0; j < i; j++)
	{
	
		if(send_string[j] == 0x0100) // Receive
		{
			send_spi_byte(0x00);
			send_string[j] = 0x0200 + SPDR;
		}
		else if(send_string[j] == 0x0101){ deselect(); }// CS HIGH
		else if(send_string[j] == 0x0102){ select(); } // CS LOW
		else if(send_string[j] == 0x0103){ delay_ms(10); } // DELAY
		else{ send_spi_byte(send_string[j]); }
	}
	deselect();
	// PRINT RESULTS
	for(j = 0; j < i; j++)
	{
	
		if(send_string[j] >= 0x0200){ printf245("R%d = 0x%x\n\r",receive,(send_string[j] & 0x00FF)); receive++; }
		else if(send_string[j] == 0x0101){ my_printf(CS_HIGH); }
		else if(send_string[j] == 0x0102){ my_printf(CS_LOW); }
		else if(send_string[j] == 0x0103){ my_printf(DELAY); }
		else{ printf245("Sent 0x%x\n\r", send_string[j]); }
	}
	my_printf(STRING_SENT);
	
	spi_menu();
}

void twi_menu(void)
{
	menu();
}

/*************************************************************************** 
	SETTINGS SECTION:       
		Settings include
			-Set clock polarity and phase
			-Change frequency
			-Change parity
****************************************************************************/

void init_settings(void)
{
	uint8_t settings_from_eeprom1 = 0;
	uint8_t settings_from_eeprom2 = 0;
	uint8_t settings_from_eeprom3 = 0;
	uint8_t settings_from_eeprom4 = 0;
	uint16_t settings_crc = 0;
	uint16_t calculated_crc = 0;
	
	settings_from_eeprom1 = eeprom_read_byte((uint8_t*)0);  // Read settings from eeprom byte 0 through 3
														   // Bits 8 through 32 are reserved, bits 5-7 contain polarity, bits 4-2 contain frequency, bit 1 contains dorder
	settings_from_eeprom2 = eeprom_read_byte((uint8_t*)1);
	settings_from_eeprom3 = eeprom_read_byte((uint8_t*)2);
	settings_from_eeprom4 = eeprom_read_byte((uint8_t*)3);
	settings_crc = eeprom_read_byte((uint8_t*)4); // Read settings crc from eeprom byte 4
	calculated_crc = _crc_ibutton_update(0, settings_from_eeprom1);
	calculated_crc = _crc_ibutton_update(calculated_crc, settings_from_eeprom2);
	calculated_crc = _crc_ibutton_update(calculated_crc, settings_from_eeprom3);
	calculated_crc = _crc_ibutton_update(calculated_crc, settings_from_eeprom4);
	if (calculated_crc != settings_crc) {
		my_printf(SETTINGS_CORRUPT); // Default to 500khz, MSB, Polarity Mode 1 0b 0001 1000
		settings_from_eeprom1 = 0x18;
		settings_from_eeprom2 = 0x0;
		settings_from_eeprom3 = 0x0;
		settings_from_eeprom4 = 0x0;
		settings_crc = 0;
		calculated_crc = _crc_ibutton_update(settings_crc, settings_from_eeprom1);
		calculated_crc = _crc_ibutton_update(calculated_crc, settings_from_eeprom2);
		calculated_crc = _crc_ibutton_update(calculated_crc, settings_from_eeprom3);
		calculated_crc = _crc_ibutton_update(calculated_crc, settings_from_eeprom4);
		eeprom_write_byte((uint8_t*)0, settings_from_eeprom1);
		eeprom_write_byte((uint8_t*)1, settings_from_eeprom2);
		eeprom_write_byte((uint8_t*)2, settings_from_eeprom3);
		eeprom_write_byte((uint8_t*)3, settings_from_eeprom4);
		eeprom_write_byte((uint8_t*)4, calculated_crc);
	} 
	if (settings_from_eeprom1 & 0x1) {
		SPCR |= (1<<DORD);
		current_dorder = '1';
		printf245("Setting LSB.\n\r");
		}
		else {
			SPCR &= ~(1<<DORD);
			current_dorder = '0';
			printf245("Setting MSB.\n\r");
			}
	settings_from_eeprom1 = settings_from_eeprom1 >> 1; // drop bit 1
	
	calculated_crc = settings_from_eeprom1 & 0x7;
	if(calculated_crc == 1|| calculated_crc == 2){ SPCR &= ~((1<<SPR0) | (1<<SPR1)); }
	else if(calculated_crc == 3|| calculated_crc == 4){ SPCR &= ~(1<<SPR1); SPCR |= (1<<SPR0); }
	else if(calculated_crc == 5|| calculated_crc == 6){ SPCR &= ~(1<<SPR0); SPCR |= (1<<SPR1); }
	else if(calculated_crc == 7){ SPCR |= ((1<<SPR0) | (1<<SPR1)); }
	if(calculated_crc == 1 || calculated_crc == 3 || calculated_crc == 5){ SPSR |= 0x01; } // Frequency doubler
	else{ SPSR &= ~(1<<SPI2X); }
	switch(calculated_crc)
	{
		case 1:
		current_frequency = '1';
		break;
		case 2:
		current_frequency = '2';
		break;
		case 3:
		current_frequency = '3';
		break;
		case 4:
		current_frequency = '4';
		break;
		case 5:
		current_frequency = '5';
		break;
		case 6:
		current_frequency = '6';
		break;
		case 7:
		current_frequency = '7';
		break;
	}
	settings_from_eeprom1 = settings_from_eeprom1 >> 3; // drop bits 2 through 4
	
	calculated_crc = settings_from_eeprom1 & 0x7;
	switch(calculated_crc)
	{
		case 1:
		SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPIE);
		current_phase = '1';
		break;
		case 2:
		SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPIE) | (1<<CPHA);
		current_phase = '2';
		break;
		case 3:
		SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPIE) | (1<<CPOL);
		current_phase = '3';
		break;
		case 4:
		SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPIE) | (1<<CPHA) | (1<<CPOL);
		current_phase = '4';
		break;
	}
}

void settings_menu(void)
{
	char c = 0;
	
	
	my_printf(SETTINGS);
	my_printf(ARROW);
	
	c = getc245_blocking();
	printf245("%c\n\r",c);
	
	switch(c)
	{
		case '1':
			set_polarity_phase();
			break;
		case '2':
			change_frequency();
			break;
		case '3':
			set_dorder();
			break;
		case '4':
			show_settings();
			break;
		case '5':
			save_settings();
			break;
		case '6':
			menu();
			break;
		default:
		{
			my_printf(INVALID);
			menu();
			break;
		}
	}
}

void set_polarity_phase(void)
{
	uint8_t c = 0;
	my_printf(SET_POLARITY_1);
	my_printf(SET_POLARITY_2);
	my_printf(SET_POLARITY_3);
	
	c = getc245_blocking();
	printf245(" %c\n\r",c);
	current_phase = c;
	switch(c)
	{
		case '1':
			SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPIE);
			break;
		case '2':
			SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPIE) | (1<<CPHA);
			break;
		case '3':
			SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPIE) | (1<<CPOL);
			break;
		case '4':
			SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPIE) | (1<<CPHA) | (1<<CPOL);
			break;
		default:
		{
			my_printf(INVALID);
			set_polarity_phase();
			break;
		}
	}
	my_printf(MODE_CHANGED);
	settings_menu();
}
	
void change_frequency(void)
{
	uint8_t c;
	my_printf(FREQUENCY_MENU_1);
	my_printf(FREQUENCY_MENU_2);
	c = getc245_blocking();
	printf245("%c\n\r",c);
	current_frequency = c;
	
	if(c == '1'|| c == '2'){ SPCR &= ~((1<<SPR0) | (1<<SPR1)); }	
	else if(c == '3'|| c == '4'){ SPCR &= ~(1<<SPR1); SPCR |= (1<<SPR0); }
	else if(c == '5'|| c == '6'){ SPCR &= ~(1<<SPR0); SPCR |= (1<<SPR1); }
	else if(c == '7'){ SPCR |= ((1<<SPR0) | (1<<SPR1)); }
	else{ my_printf(INVALID); }

	if(c == '1' || c == '3' || c == '5'){ SPSR |= 0x01; } // Frequency doubler
	else{ SPSR &= ~(1<<SPI2X); } 
	
	my_printf(FREQUENCY_CHANGED);
	settings_menu();
		
}

void set_dorder(void)
{
	uint8_t c;
	my_printf(DORDER_MENU);
	c = getc245_blocking();
	printf245("%c\n\r",c);
	current_dorder = c;
	if(c){ SPCR |= (1<<DORD); }
	else{ SPCR &= ~(1<<DORD); }
	my_printf(DORDER_CHANGED);
	settings_menu();
}

void show_settings(void)
{	
	my_printf(CURRENT_SETTINGS);
	printf245("Polarity/Phase Mode: %c\n\r", current_phase);
	printf245("Clock frequency: ");
	switch(current_frequency)
	{
		case '1': printf245("4MHz\n\r"); break;
		case '2': printf245("2MHz\n\r"); break;
		case '3': printf245("1MHz\n\r"); break;
		case '4': printf245("500kHz\n\r"); break;
		case '5': printf245("250kHz\n\r"); break;
		case '6': printf245("125kHz\n\r"); break;
		case '7': printf245("62.5kHz\n\r"); break; 
		default:
		printf245("Frequency: %c\n\r", current_frequency);
	}
	my_printf(DATA_ORDER);
	if(current_dorder == '1'){ my_printf(LSB); }
	else{ my_printf(MSB); }
	settings_menu();
}

void save_settings(void)
{
		uint8_t settings_from_eeprom1 = 0;
		uint8_t settings_from_eeprom2 = 0;
		uint8_t settings_from_eeprom3 = 0;
		uint8_t settings_from_eeprom4 = 0;
		uint16_t settings_crc = 0;
		
		// Code to figure out settings_from_eeprom1
		/* settings_from_eeprom1 = current_dorder + (current_frequency << 1) + (current_phase << 4); All wrong.  Stupid ascii. */
		if (current_dorder == '1') {settings_from_eeprom1 = 1;}
			else {settings_from_eeprom1 = 0;}
		
		switch(current_frequency)
		{
			case '1':
			settings_from_eeprom1 += (1 << 1);
			break;
			case '2':
			settings_from_eeprom1 += (2 << 1);
			break;
			case '3':
			settings_from_eeprom1 += (3 << 1);
			break;
			case '4':
			settings_from_eeprom1 += (4 << 1);
			break;
			case '5':
			settings_from_eeprom1 += (5 << 1);
			break;
			case '6':
			settings_from_eeprom1 += (6 << 1);
			break;
			case '7':
			settings_from_eeprom1 += (7 << 1);
			break;
		}
		
		switch(current_phase)
		{
			case '1':
			settings_from_eeprom1 += (1 << 4);
			break;
			case '2':
			settings_from_eeprom1 += (2 << 4);
			break;
			case '3':
			settings_from_eeprom1 += (3 << 4);
			break;
			case '4':
			settings_from_eeprom1 += (4 << 4);
			break;
		}
		
		// Calculates CRC, then writes out the settings + CRC
		settings_crc = 0;
		settings_crc = _crc_ibutton_update(settings_crc, settings_from_eeprom1);
		settings_crc = _crc_ibutton_update(settings_crc, settings_from_eeprom2);
		settings_crc = _crc_ibutton_update(settings_crc, settings_from_eeprom3);
		settings_crc = _crc_ibutton_update(settings_crc, settings_from_eeprom4);
		eeprom_write_byte((uint8_t*)0, settings_from_eeprom1);
		eeprom_write_byte((uint8_t*)1, settings_from_eeprom2);
		eeprom_write_byte((uint8_t*)2, settings_from_eeprom3);
		eeprom_write_byte((uint8_t*)3, settings_from_eeprom4);
		eeprom_write_byte((uint8_t*)4, settings_crc);
		settings_menu();
}

char getc245_blocking()
{
	char c = 0;
	while(c == 0){ c = getchar245(); }
	return c;
}

void my_printf(uint8_t string_num)
{
	strcpy_P(buffer, (PGM_P)pgm_read_word(&(string_table[string_num])));
	printf245(buffer);
}

