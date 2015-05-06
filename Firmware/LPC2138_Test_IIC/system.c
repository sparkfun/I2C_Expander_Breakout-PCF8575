/* system support for 
 * LPC2138
 */
/* David Wolpoff 
 * Last modified July 2006
 *
 * This thing has been hacked so many times, I don't really even
 * want to know what it does and DOES NOT do. 
 * Nothing here is meant to be particularly innovatinve
 * or useful, unless you happen to have an Olimex LPC2138 board.
 *
 * I'm not taking the time to re-comment this. Sorry.
 *
 * Based on system.c from Owen Osborn, SFE.
 * */
#include "printmacros.h"
#include "system.h"
#include "intcomm.h"
#include "LPC214x.h"
#include <stdio.h>

int printidx;
char printbuf[128];


/* Define catchall routines for vector table */
void IRQ_Routine (void)   __attribute__ ((interrupt("IRQ")));
void IRQ_Routine (void)
{
  PRINTF0("IRQ\n\r");
  while(1);
}

void FIQ_Routine (void)   __attribute__ ((interrupt("FIQ")));
void FIQ_Routine (void)
{
  PRINTF0("FIQ\n\r");
  while(1);
}

void SWI_Routine (void)   __attribute__ ((interrupt("SWI")));
void SWI_Routine (void)
{
  PRINTF0("SWI\n\r");
  while(1);
}

void UNDEF_Routine (void) __attribute__ ((interrupt("UNDEF")));
void UNDEF_Routine (void)
{
  PRINTF0("UNDEF\n\r");
  while(1);
};

int timer_count = 0;

static void timer0ISR(void) __attribute__ ((interrupt ("IRQ")));
static void timer0ISR(void)
{
  static int led_state1 = 1;
  static int led_state2 = 1;
  timer_count++;
  timer_count %= TIME_PERIOD;

  if((timer_count)==0)
  {
    /*    led_state1 ^= 1;*/
    led_state2 ^= 1;
    /*    led1(led_state1);*/
    led2(led_state2);
  }

  /* Clear the timer 0 interrupt */
  T0IR = 0xFF;
  /* Update VIC priorities */
  VICVectAddr = 0;

}

// system putchar called by printf
void __putchar(char c)
{
  // RS-232 section
  if (c == '\n')
    put_char('\r');
  put_char(c);
  return;
}

char * hexify(int in)
{
  static char hex[11];
  hex[0]='0';
  hex[1]='x';

  int i;
  for(i = 0; i < 8; i++)
  {
    hex[9-i]=0x30+(in&0x0f);
    if(hex[9-i] > '9')
      hex[9-i] += ('A' - '9')-1;
    in=in>>4;
  }
  hex[10]='\0';
  return hex;
  
}

void __putstr(char*c,int len)
{
  int cnt;
  for(cnt = 0; cnt < len; cnt++)
  __putchar(*c++);
}


// Write this
int __getchar()
{
  return 0;
}

// a stupid delay
void delay_ms(int count)
{
  int i;
  count *= 2600;
  for (i = 0; i < count; i++)
  {
    asm volatile ("nop");
  }
  return;
}

//Sends character to the Transmit Register
void put_char(char c) 
{
  while(uart0_putch(c)==-1);
  return;
}

void catch_errors(void)
{
  /* Put whatever error conditions here */

}

void boot_up(void){
  // Initialize the MCU clock PLL
  system_init();
  delay_ms(200);


  /* Initialize Timer 0 Interrupt at 100Hz */

  /* Timer 0 interrupt is an IRQ interrupt */
  VICIntSelect &= ~0x10;
  /* Enable timer 0 interrupt */
  VICIntEnable = 0x10;
  /* Use slot 0 for timer 0 interrupt */
  VICVectCntl1 = 0x24;
  /* Set the address of ISR for slot 0 */
  VICVectAddr1 = (unsigned int)timer0ISR;

  /* Reset timer 0 */
  T0TCR = 0;
  /* Set the timer 0 prescale counter */
  T0PR = 0;
  /* Set timer 0 match register */
  T0MR0 = 589824;
  /* Generate interrupt and reset counter on match */
  T0MCR = 3;
  /* Start timer 0 */
  T0TCR = 1;

  /* End Timer 0 Initialization */

  delay_ms(20);

  /* Init Subsystems/modules here */
  leds_init();
  buttons_init();
  uart0_init();
  uart0ISR_init();

  /* Enable Interrupts */
  /* __ARMLIB_enableIRQ();*/
}

void system_init(void)  {


  // Setting Multiplier and Divider values
  PLLCFG=0x23;
  feed();

  // Enabling the PLL */	
  PLLCON=0x1;
  feed();

  // Wait for the PLL to lock to set frequency
  while(!(PLLSTAT & PLOCK)) ;

  // Connect the PLL as the clock source
  PLLCON=0x3;
  feed();

  // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
  MAMCR=0x2;
  MAMTIM=0x4;

  // Setting peripheral Clock (pclk) to System Clock (cclk)
  VPBDIV=0x1;

}


void feed(void)
{
  PLLFEED=0xAA;
  PLLFEED=0x55;
}


/* Routines LED/BUTTON on Olimex LPC2138 Proto Board */

void leds_init()
{
  IODIR0 |= ((1 << 12) | (1 << 13));
}

void led1(int state)
{
  if(state)
  {
    IOSET0 |= (1 << 12);
  }
  else
  {
    IOCLR0 |= (1 << 12);
  }
}

void led2(int state)
{
  if(state)
  {
    IOSET0 |= (1 << 13);
  }
  else
  {
    IOCLR0 |= (1 << 13);
  }
}

void buttons_init()
{
  IODIR0 &= ~((1 << 15)|(1 << 16));
}

int button1()
{
  if((IOPIN0 & (1 << 15)))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

int button2()
{
  if((IOPIN0 & (1 << 16)))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
