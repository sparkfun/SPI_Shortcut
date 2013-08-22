#include <inttypes.h>


void ioinit (void);
void delay_ms(uint16_t x);
void delay_micro(uint16_t x);
void setDataOutputs(void);
void setDataInputs(void);
void printf245(const char *text, ...);
char getchar245(void);
void reverse(char s[]);
void itoa2(int n, char s[]);
char string_compare(const char *search_string, const char *find_string);
void ascii2hex(volatile char *ascii_string);
