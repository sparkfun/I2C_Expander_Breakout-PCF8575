/* intcomm.c
 * Interrupt Driven Serial IO via uart0
 * David Wolpoff
 * 6/8/06
 *
 * Modified to use FIQ 7 September 2006 
 */
#include "LPC214x.h"
#include "circbuf.h"
#include "system.h"

volatile buf_type rxbuf;
volatile buf_type txbuf;

void uart0ISR(void) __attribute__ ((interrupt ("FIQ")));

void uart0ISR(void)
{
  unsigned char iir,tmp;

  iir = U0IIR;

  /* Check that we have an interrupt 
   * and process until we don't */
  while(!(iir&0x01))
  {

    switch((iir >> 1)&0x07)
    {
      case 1: /* THRE */
        if(bufused(&txbuf))
        {
          /*          if we have a character, extract and put in thr*/
          U0THR = (unsigned char)bufextract(&txbuf);
          if(bufused(&txbuf)==0)
          {
            U0IER &= ~0x02;
          }
        }
        else
        {
          /*          otherwise we're screwed. turn of thre int*/
          U0IER &= ~0x02;
        }
        break;
      case 2: /* RDA */
        /* Don't bother checking for room, we've got to take the
         * thing anyway. */
        tmp = U0RBR;
        bufinsert(&rxbuf,(unsigned int)tmp);
        break;
      case 3: /* RLS */
        tmp = U0LSR;
        break;
      case 6: /* CTI */
        break;
      default:
        break;
    }
    iir = U0IIR;
  }

  /* Update VIC priorities */
  VICVectAddr = 0;
}

void uart0ISR_init(void)
{
  /* Initialize the buffers */
  init_buffer(&rxbuf);
  init_buffer(&txbuf);

  /* Set UART 0 interrupt as FIQ */
  VICIntSelect |= 0x040;

  /* Use slot 2 for uart0 interrupt */
  VICVectCntl2 = 0x26;

  /* Set vector address */
  VICVectAddr2 = (unsigned int)uart0ISR;

  /* Enable Interrupt */
  VICIntEnable = 0x040;


  /* Enable RX interrupts in UART0 */
  U0IER = 0x01;

}

void uart0_init()
{
  char tmp;
  PINSEL0 |= 0x00000005;	//enable uart0
  //Divisor = Pclk/(16*baudrate), where Pclk = VPBclk = Cclk/VPBDIV
  U0LCR 	= 0x83;			// 8 bits, no Parity, 1 Stop bit, DLAB = 1 
  U0DLM   = 0x01;		  	//
  U0DLL 	= 0x80;        	// 9600 Baud Rate @ 58982400 VPB Clock  
  U0LCR 	= 0x03;        	// DLAB = 0   
  tmp = U0THR;
  U0RBR = 0;
}

void uart0ISR_stop(void)
{
  /* Initialize the buffers */
  init_buffer(&rxbuf);
  init_buffer(&txbuf);

  /* Disable Interrupt */
  VICIntEnClr = 0x040;


  /* Disable all interrupts in UART0 */
  U0IER = 0x00;
}

int uart0_putch(unsigned char input)
{
  if( bufinsert(&txbuf,(unsigned int)input) != -1)
  {
    /* If we put something in the buffer, 
     * make sure interrupt to send is on 
     */
    U0IER |= 0x02;
    return 0;
  }
  return -1;
}

unsigned char uart0_getch()
{
  if(bufused(&rxbuf) == 0 )
    return 0;
  return (unsigned char)bufextract(&rxbuf);
}
