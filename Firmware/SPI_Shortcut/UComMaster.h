//*******************************************************
//
//		   SPI Shortcut Pin Definitions 
//				SparkFun Electronics
//   		   Written By Chris Taylor
//			
//*******************************************************
//HARDWARE NOTE: High Fuse should be set to 0xDF
//				 Low Fuse should be set to 0xFF

//*******************************************************
//					General Definitions
//*******************************************************
#define HOLD_TIME	20
#define OK	1
#define ERROR	0

#define CS		PB2
#define MOSI	PB3
#define MISO 	PB4
#define SCK		PB5

#define LED		PB1

#define SCL 	PC5
#define SDA		PC4


//*******************************************************
//					ATmega168 Pin Definitions
//*******************************************************

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

//SPI Communication Pins
#define SPI_SCLK	5
#define SPI_IN		4
#define SPI_OUT		3
#define SPI_CS		2

//TWI Communications Pins
#define TWI_SCL 5
#define TWI_SDA 4

//FT245 Control Pin Definitions
#define PWREN	(1<<0) //PB0 (Input to AVR)
#define RD		(1<<0) //PC0 (Output from AVR)
#define WR		(1<<1) //PC1 (Output from AVR)
#define TXE		(1<<2) //PC2 (Input to AVR)
#define RXF		(1<<3) //PC3 (Input to AVR)

//FT245 Data Pins (These pins switch between being I/O)
#define D0		(1<<0) //PD0
#define D1		(1<<1) //PD1
#define D2		(1<<2) //PD2
#define D3		(1<<3) //PD3
#define D4		(1<<4) //PD4
#define D5		(1<<5) //PD5
#define D6		(1<<6) //PD6
#define D7		(1<<7) //PD7

//*******************************************************
//					Global Macros
//*******************************************************
#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

//Macros for interfacing with the FT245
#define USBreadAllowed()	(PINC & RXF) != RXF
#define USBwriteAllowed()  	(PINC & TXE) != TXE
#define USBConfigured() 	(PINB & PWREN) != PWREN
#define setRD()		PORTC |= RD;
#define clearRD()	PORTC &= ~RD;
#define setWR()		PORTC |= WR;
#define clearWR()	PORTC &= ~WR;

//*******************************************************
//					Global Functions
//*******************************************************
void delay_ms(uint16_t x); // general purpose delay
void delay_micro(uint16_t x); // general purpose delay
void fill16(volatile uint8_t* src, uint8_t fill);
void fillFrame(volatile uint8_t* src, uint8_t fill);
void newcolor(uint8_t * r, uint8_t * g, uint8_t * b);
void clock_byte(volatile uint16_t my_byte, volatile uint8_t rowNum);
void ioinit (void);      // initializes IO
void setDataOutputs(void);
void setDataInputs(void);
void printf245(const char *text, ...);
char getchar245(void);
void reverse(char s[]);
void itoa2(int n, char s[]);
void parseCommand(void);
void executeCommand(void);
char string_compare(const char *search_string, const char *find_string);
void ascii2hex(volatile char *ascii_string);
void setPixel(int board, int offset, char r, char g, char b);
void setBoard(char r, char g, char b);
void sendButtonValues(void);
unsigned char read_from_EEPROM(unsigned int Address);
void write_to_EEPROM(unsigned int Address, unsigned char Data);

//*******************************************************
//					Structure Definitions
//*******************************************************
typedef struct{
	char ID;
	char Parameters[14];
	//char Checksum;
} Packet;

//*******************************************************
//					my_printf definitions 
//*******************************************************
#define MAIN_MENU 0
#define ARROW 1
#define INVALID 2
#define SEND_SINGLE 3 
#define ACTIONS_MENU 4
#define CONT_RECEIVE 5
#define SEND_COMMAND 6
#define CS_HIGH 7
#define CS_LOW 8 
#define STRING_SENT 9
#define SETTINGS 10
#define SET_POLARITY_1 11
#define SET_POLARITY_2 12
#define SET_POLARITY_3 13
#define MODE_CHANGED 14
#define FREQUENCY_MENU_1 15
#define FREQUENCY_MENU_2 16
#define FREQUENCY_CHANGED 17
#define DORDER_MENU 18
#define DORDER_CHANGED 19
#define CURRENT_SETTINGS 20
#define DATA_ORDER 21
#define MSB 22
#define LSB 23
#define DELAY 24
#define SETTINGS_CORRUPT 25
#define UNIMPLEMENTED 26
#define TWI_ACTION_MENU 27
