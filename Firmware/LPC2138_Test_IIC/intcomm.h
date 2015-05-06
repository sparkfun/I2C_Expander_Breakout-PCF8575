/* intcomm.h
 * Interrupt Driven Serial IO via uart0
 * David Wolpoff
 * 6/6/06
 *
 * Modified to FIQ 7 September 2006
 */

void uart0ISR_init(void);
void uart0ISR_stop(void);
void uart0_init();

/* Returns -1 if the thing failed, 0 otherwise */
int uart0_putch(unsigned char input);

/* always returns a character--
 * returns 0 if the rxbuf was empty 
 */
unsigned char uart0_getch();

