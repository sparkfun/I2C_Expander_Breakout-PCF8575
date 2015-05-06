/* I2C interface for LPC2138 */
/* David Wolpoff */

void i2c0ISR_init();
void i2c0_init();
int i2c0_master_send(unsigned char addy, unsigned char* ptr, int count);
int i2c0_master_receive(unsigned char addy, unsigned char* ptr, int count);
int i2c0_master_send_receive(unsigned char addy, unsigned char* ptr, int count, int count2);
int send_char_iic(unsigned char addy, unsigned char data);
