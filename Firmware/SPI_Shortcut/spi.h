
void spi_init(void);
void select(void);
void reselect(void);
void deselect(void);
void send_spi_byte(char c);
char read_spi_byte(void);
