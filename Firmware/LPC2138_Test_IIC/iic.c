/* I2C interface for LPC2138 */
/* David Wolpoff */

#include "LPC214x.h"
#include "system.h"
#include <stdio.h>
#include "printmacros.h"

volatile int mdc;
volatile unsigned char* mptr;
volatile unsigned char addy_byte;
volatile int mdc2;
volatile unsigned char* mptr2;
volatile unsigned char addy_byte2;
volatile int xfer_done;
volatile int xfer_status;

void i2c0ISR(void) __attribute__ ((interrupt ("IRQ")));

void i2c0ISR(void)
{
  unsigned char status;

  status = I20STAT;


  switch(status)
  {
    default:
    case 0x00:
/*      __putstr("0x00\n\r",6);*/
      I20CONSET = 0x14;
      I20CONCLR = 0x08;
      xfer_status = 1;
      xfer_done = 1;
      break;
    case 0x08: /* Start sent */
/*      __putstr("0x08\n\r",6);*/
    case 0x10: /* now send addy */
/*      __putstr("0x10\n\r",6);*/
      I20DAT = addy_byte;
/*      PRINTF1("ADDY BYTE: %X\n\r",addy_byte);*/
      I20CONSET = 0x04;
      I20CONCLR = 0x28;
      break;
    case 0x18: /* Addy sent, ack received */
/*      __putstr("0x18\n\r",6);*/
      I20DAT = *mptr;
/*      PRINTF1("send %X\n\r",*mptr);*/
      mptr++;
      I20CONSET = 0x04;
      I20CONCLR = 0x08;
      break;
    case 0x20: /* Addy sent, no ack, stop.*/
/*      __putstr("0x20\n\r",6);*/
      I20CONSET = 0x14;
      I20CONCLR = 0x28;
      xfer_done = 1;
      xfer_status = 1;
      break;
    case 0x28: /* Data sent, ack received */
/*      __putstr("0x28\n\r",6);*/
      mdc--;
      if(mdc == 0)
      {
        if(mptr2 == NULL)
        {
          I20CONSET = 0x14;
          I20CONCLR = 0x08;
          xfer_done = 1;
          xfer_status = 0;
          break;
        }
        else
        {
          addy_byte = addy_byte2;
          mptr = mptr2;
          mdc = mdc2;
          mptr2 = NULL;

          I20CONCLR = 0x08;
          I20CONSET = 0x20; /* Repeat start */
          break;
        }
      }
      I20DAT = *mptr++;
      I20CONSET = 0x04;
      I20CONCLR = 0x08;
      break;
    case 0x30: /* Data sent, no ack, stop*/
/*      __putstr("0x30\n\r",6);*/
      I20CONSET = 0x14;
      I20CONCLR = 0x08;
      xfer_done = 1;
      xfer_status = 0;
      break;
    case 0x38: /* Arbitration lost
                  bus released.
                  New start to be sent
                  when bus is free */
/*      __putstr("0x38\n\r",6);*/
      I20CONSET = 0x24;
      I20CONCLR = 0x08;
      break;
    case 0x40: /* Addy+read sent, ack received */
/*      __putstr("0x40\n\r",6);*/
      I20CONCLR = 0x08;
      if(mdc!=1)
      {
        I20CONSET = 0x04;
      }
      else
      {
        I20CONCLR = 0x04;
      }
      break;
    case 0x48: /* Addy+read sent, no ack, send stop */
/*      __putstr("0x48\n\r",6);*/
      I20CONSET = 0x14;
      I20CONCLR = 0x08;
      xfer_done = 1;
      xfer_status = 1;
      break;
    case 0x50: /* Data received, ack returned */
/*      __putstr("0x50\n\r",6);*/
      *mptr = I20DAT;
/*      PRINTF1("read %X\r\n",*mptr);*/
      mptr++;
      mdc--;
      if(mdc==1)
      {
        I20CONCLR = 0x0C;
      }
      else
      {
        I20CONSET = 0x04;
        I20CONCLR = 0x08;
      }
      break;
    case 0x58: /* Data received, no ack. Send stop */
/*      __putstr("0x58\n\r",6);*/
      *mptr = I20DAT;
/*      PRINTF1("read %X\n\r",*mptr);*/
      if(mptr2 == NULL)
      {
        I20CONSET = 0x14;
        I20CONCLR = 0x08;
        xfer_done = 1;
        xfer_status = 0;
      }
      else
      {
        addy_byte = addy_byte2;
        mptr = mptr2;
        mdc = mdc2;
        mptr2 = NULL;

        I20CONCLR = 0x08;
        I20CONSET = 0x20; /* Repeat start */
      }
      break;
  }


  /* Update VIC priorities */
  VICVectAddr = 0;
}

void i2c0_init()
{
  /* Enable i2c pins */
  PINSEL0 |= (1 << 4) | (1 << 6);

  I20CONCLR = 0x6C;

  /* Set Timing */
  /* Input MHz * Multiplier / Divider / iicfreq+1 / 2 */
  I20SCLH = (58000000 / 100001) / 2;
  /* Input MHz * Multiplier / Divider / iicfreq / 2 */
  I20SCLL = (58000000 / 100000) / 2;
  
  I20ADR = 0;

  /* Enable I2C interface */
  I20CONSET = (1 << 6);
}


void i2c0ISR_init(void)
{
  /* Set I2C0 interrupt as IRQ */
  VICIntSelect &= ~0x200;

  /* Use level 3 for I2C interrupt */
  VICVectCntl3 = 0x29;

  /* Set vector address */
  VICVectAddr3 = (unsigned int)i2c0ISR;

  /* Enable Interrupt */
  VICIntEnable = 0x200;

}

int i2c0_master_send(unsigned char addy, unsigned char* ptr, int count)
{
/*  PRINTF0("Enter Send\n\r");*/
  mdc = count;
  mptr = ptr;
  /* Slave addr. is 7 bits, plus lsb 0 for write */
  addy_byte = (addy) | 0;

  xfer_done = 0;
  xfer_status = 0;

  /* Set Start Condition Bit (start) */
  I20CONSET = 0x20;

  while(xfer_done==0)
  {
/*  PRINTF0(".");*/
/*  delay_ms(500);*/
  }
/*  PRINTF0("Exit Send\n\r");*/
  return xfer_status;
}

int i2c0_master_receive(unsigned char addy, unsigned char* ptr, int count)
{
/*  PRINTF0("Enter receive\n\r");*/
  xfer_done = 0;
  xfer_status = 0;
  mdc = count;
  mptr = ptr;

  /*Slave addr. is 7 bits, plus lsb 1 for read */
  addy_byte = addy | 1;
  
  /* Set Start Condition (start) */
  I20CONSET = 0x20;

  while(xfer_done==0)
  {
  }
/*  PRINTF0("Exit receive\n\r");*/
  return xfer_status;

}

int i2c0_master_send_receive(unsigned char addy, unsigned char* ptr, int tx_count, int rx_count)
{
  xfer_done = 0;
  xfer_status = 0;
  mdc = tx_count;
  mptr = ptr;

  mdc2 = rx_count;
  mptr2 = ptr;
  
  addy_byte = addy;
  addy_byte2 = addy | 1;
  
  I20CONSET = 0x20;

  while(xfer_done == 0);

  return xfer_status;
}
int send_char_iic(unsigned char addy, unsigned char data)
{
  unsigned char t;

  /* Send start */
  I20CONSET = 0x20;

  /* Wait for start sent */
  while(I20STAT != 0x08);

  /* Write addy to data reg */
  I20DAT = addy;
  I20CONSET = 0x04;
  I20CONCLR = 0x08;

  /* wait for a state change, */
  t = I20STAT;
  while((t != 0x18) && (t != 0x20))
    t = I20STAT;

  if(t==0x20) /* Nobody home. Return error */
  {
    I20CONSET = 0x10;
    I20CONCLR = 0x08;
    PRINTF0("Nobody home\n\r");
    return 1;
  }

  if(t==0x18) /* Got ack. send data */
  {
    I20DAT = data;
    I20CONSET = 0x04;
    I20CONCLR = 0x08;
  }

  /* Wait for a state change */
  t = I20STAT;
  while((t!=0x28) && (t!=0x30))
    t = I20STAT;

  if(t == 0x30)
  {
    I20CONSET = 0x10;
    I20CONCLR = 0x08;
    return 0;
  }

  if(t == 0x28)
  {
    I20CONSET = 0x14;
    I20CONCLR = 0x08;
    return 0;
  }
}
