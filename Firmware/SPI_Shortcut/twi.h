/*
 * twi.h
 *
 * Created: 8/16/2013 10:22:44 PM
 *  Author: nick
 */ 


#ifndef TWI_H_
#define TWI_H_
void tw_init(void);
void send_twi_byte(char c);
char read_twi_byte(void);
void twi_start(void);
void twi_stop(void);
void twi_write(uint8_t i);
uint8_t twi_readACK(void);
uint8_t twi_readNACK(void);
uint8_t twi_getstatus(void);


#endif /* TWI_H_ */