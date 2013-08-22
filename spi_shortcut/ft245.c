#include <inttypes.h>
#include <ctype.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "UComMaster.h"

//Initializes the I/O pins, the interrupts and the system parameters
void ioinit (void)
{
    //1 = output, 0 = input   
	DDRC |= WR | RD;
	DDRC &= ~(RXF | TXE);
	DDRB &= ~(PWREN);
	PORTC |= (RXF | TXE);
	PORTB |= (PWREN);
	
	DDRD &= ~(D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7);
	PORTD |= (D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7);
	
}   

void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0;  y < 40 ; y++){
      for ( z = 0 ; z < 40 ; z++){
        asm volatile ("nop");
      }
    }
  }
}

//General short delays
void delay_micro(uint16_t x)
{
  uint8_t y;
  for ( ; x > 0 ; x--){
    for ( y = 0;  y < 4 ; y++){
		asm volatile ("nop");
    }
  }
}

//Configures the I/O pins to output to the FT245
void setDataOutputs(void){
    //1 = output, 0 = input
	PORTD &= ~(D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7); //Disable Pullups (Intermediate Step)
	DDRD |= (D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7);
}

//Configures the I/O pins to input from the FT245
void setDataInputs(void){
    //1 = output, 0 = input	
	DDRD &= ~(D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7);
	PORTD |= (D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7); //Enable Pullups
}

//Printf a string to the FT245. Works like printf
void printf245(const char *text, ...){
	char character=0, temp=0;
	char decimal[6];
	int index=0;
	
	va_list next_argument;		//Create a pointer for the variable argument
	va_start(next_argument, text);	//Initialize the pointer to the next argument in the list
	
	setDataOutputs();
	character=text[index];
	while(character!='\0'){
		if(USBwriteAllowed()){
			if(character=='%'){	//Need to access an argument value!
				character = (char)va_arg(next_argument, int);	//Get the value of the current argument
				
				if(text[index+1]=='d'){
					itoa2(character, decimal);
					for(int i=0; decimal[i]!='\0'; i++)printf245("%c", decimal[i]);
					index+=1;
				}
				else if(text[index+1]=='x'){
					temp=character;
					temp=temp>>4;
					if(temp>9)temp+=('A'-10);
					else temp+='0';
					printf245("%c", temp);
					character=character&0x0f;
					if(character>9)character+=('A'-10);
					else character+='0';
					printf245("%c", character);
					index+=1;
				}
				else if(text[index+1]=='c'){
					PORTD = character;				
					clearWR();
//					delay_ms(1);
					setWR();
					index+=1;
				}				
			}
			else{
				PORTD = character;				
				clearWR();
//				delay_ms(1);
				setWR();
			}
			character=text[++index];
		}
	}
	va_end(next_argument);	//Relinquish the pointer to system memory
}

//Returns a character from the FT245
char getchar245(void){
	char character=0;
	
	setDataInputs();
	if(USBreadAllowed()){		//If RX Flag is low, lets get the data from the FIFO
		clearRD();			//Clock the next FIFO data onto the D0-D7 lines
		delay_micro(HOLD_TIME);
		character = PIND;		//Get data from FT245		
		setRD();			//Relinquish control of the Data lines
	}
	
	return character;
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert n to characters in s */
void itoa2(int n, char s[])
{
    int i=0;

    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */

    s[i] = '\0';
    reverse(s);
}



//Use to find a string within a search string
//search : "hithere\0"
//find : "the\0" - return OK
//***************************** UNTESTED *********************************
char string_compare(const char *search_string, const char *find_string)
{

    int find_spot, search_spot;
    char spot_character, search_character;
    find_spot = 0;

    for(search_spot = 0 ; ; search_spot++)
    {
        if(find_string[find_spot] == '\0') return OK; //We've reached the end of the search string - that's good!
        if(search_string[search_spot] == '\0') return ERROR; //End of string found

        spot_character = find_string[find_spot]; //Compiler limit
        search_character = search_string[search_spot]; //Compiler limit

        if(spot_character == search_character) //We found another character
            find_spot++; //Look for the next spot in the search string
        else if(find_spot > 0) //No character found, so reset the find_spot
            find_spot = 0;
    }

    return 0;
}




void ascii2hex(volatile char *ascii_string)
{
	char hex_value;
	char hex_string[20];
	int hex_size=0;
	int ascii_index=1;
	
	//Convert each character in the received command buffer to a hex value
	for(int string_index=1; ascii_string[string_index] != '\n'; string_index++){
		hex_value=ascii_string[string_index];
		if(isdigit(hex_value))hex_value-='0';
		else if(isupper(hex_value)){
			hex_value-='A';
			hex_value+=10;
		}
		else if(islower(hex_value)){
			hex_value-='a';
			hex_value+=10;
		}
		hex_string[string_index-1]=hex_value;
		hex_size++;
	}
	hex_string[hex_size]='\n';

	//Now that we have the hex values, we need to concatonate each "pair" since we received the command serially.
	//(i.e. a received command: "01A23E" is 6 characters, but the hex command is only 3 characters)
	for(int index=0; hex_string[index]!='\n'; index++){
		
		//Combine two values in the hex string to form a single hex byte
		hex_value=((hex_string[index]&0x0f)<<4) | (hex_string[index+1]&0x0f);
		
		//Put the new hex value back in the ascii string for use by the main program
		ascii_string[ascii_index++]=hex_value;
		
		//Increment the hex index since we are only getting "pairs"
		index+=1;
		hex_size=index;
	}
	ascii_index=1;
	ascii_string[hex_size/2+2]='\n';
	
}


