/* System support for LPC2138 */
/* David Wolpoff 
 * Owen Osborn */
/* Last modified July 2006 */

#define TIME_PERIOD 50

#define PLOCK 0x400


// these are for setting up the LPC clock for 4x PLL
void system_init(void);
void feed(void);

// general purpose
void delay_ms(int);

// general communications functions
void __putchar(char c);
void put_char(char c);
void __putstr(char *c, int cnt);

void boot_up(void);

/* Routines for LED/BUTTON on Olimex LPC2138 proto */
char* hexify(int blah);
void leds_init();
void buttons_init();
void led1(int state);
void led2(int state);
int button1();
int button2();
