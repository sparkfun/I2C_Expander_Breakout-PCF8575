/* David Wolpoff (david AT sparkfun DOT com)
 * Last modified 7 September 2006 
 *
 * I2C Example For LPC2138 and PCF8575, HMC6352, AD7746 
 * 
 */


#include <stdio.h>
#include "LPC214x.h"
#include "system.h"
#include "intcomm.h"
#include "iic.h"
#include "printmacros.h"

/* Function Prototypes */
void PCF8575_breakout(void);
void HMC6352_breakout(void);
void AD7746_breakout(void);

/* Number of functions in current test rig */
#define NUM_FUNCS 3

/* Function pointer array */
void (*funcs[NUM_FUNCS])(void) = {
PCF8575_breakout,
HMC6352_breakout,
AD7746_breakout
};

/* IIC Address array
 * PLEASE NOTE!
 * These addresses are in 8 bit format!
 * If your device assumes 7-bit i2c addresses,
 * you should take its address and shift left
 * one position.
 * The tx/rx functions will automagically
 * add the R/W bit
 */
char r_addy[NUM_FUNCS] = { 
0x40, 
0x42,
0x90
};

/* Device Names array */
char * names[NUM_FUNCS] = {
"PCF8575",
"HMC6352",
"AD7746"
};

int main(void)
{
  int v=0;
  unsigned char bs=0;
  int p=0;
  unsigned char data[2]={0,0};

/* Do basic initialization */
  boot_up(); /* From system.c */

/* Initialize IIC Module */
  i2c0_init();  /* From iic.c */
  i2c0ISR_init(); /* From iic.c */

  /* There IS a method to the madness!
   * PRINTF# macros are used to wrap sprintf calls
   * as output routines. This is done to reduce 
   * calls to malloc and avoid needing to fflush(stdout)
   * in order for output to show up.
   * This is because I am too lazy to rebuild newlib yet again
   */
  PRINTF0("I2C started\n\r");


  /* Forever!
   * Spin through the list of devices, announcing which device we're
   * currently attempting to communicate with. Whenever we
   * are successful in writing to a device (meaning the device gave
   * an acknowledge on the I2C interface) we call that device's 
   * routine. 
   * Basically, that means that if the iic.c routine doesn't throw an
   * error for a given device, we assume that device is working,
   * and we call its function.
   * Note that I don't actually expect any of the routines to return.
   * This loop is infinite because A. I've been taught to never let
   * anything go to the weeds. B. We use this code as a testbed, 
   * and it's not necessarily true that the device is connected prior
   * to reset.
   */
  while(1)
  {
    PRINTF1("Trying %s\n\r",names[p]);
    if(i2c0_master_send(r_addy[p],data,1) == 0) /* i2c0 routines
                                                   come from iic.c */
    {
      PRINTF1("Found %s\n\r",names[p]);
      funcs[p](); /* Call the function from the function pointer table */
    }
    p = (p+1)%NUM_FUNCS; /* Roll through a loop */
    delay_ms(1000); /* But pause for ~1 second */
  }
}

void PCF8575_breakout(void)
{
  int v=0;
  unsigned char bs=0;
  int p=0;
  unsigned char data[2]={0,0};

  while(1)
  {
    /* We test a case where we have a PCF8575 i2c bus expander
     * breakout board, and have resistors+leds connected from VCC
     * to P05 P06 P07
     * We make those LED's blink incrementing binary values.
     */
    p = (p+1)&7;  /* Calc number */
    data[0]=p<<5; /* Shift to the rigth position */
    if(i2c0_master_send(0x40,data,2)) /* Send the data to the device */
    {
      PRINTF0("Error communicating with PCF8578\n\r"); /* Self explanatory */
    }
    delay_ms(1000); /* Otherwise pause, and tick to next number */
  }
}

void HMC6352_breakout(void)
{
  int v=0;
  unsigned char bs=0;
  int p=0;
  unsigned char data[2]={0,0};
  while(1)
  {
    /* We just want to know that the HMC6352 is working
     * and generally points us in the right direction
     */
    data[0]='A'; /* Send a "Get Data" command to the compass */
    if(i2c0_master_send(0x42,data,1))
    {
      PRINTF0("HMC6352 Communication Error!\n\r"); /* Self explanatory */
    }
    if(i2c0_master_receive(0x42,data,2)==0) /* read out the two bytes of data */
    {
      /* Compute and print out a heading.
       * Note the conversion to floating point and 0.1 deg accuracy */
      PRINTF1("Heading: %3.1fdeg\n\r",((data[0] << 8)+data[1])/10.0); 
      delay_ms(500); /* Wait to avoid spamming too much */
    }
    else
    {
      PRINTF0("Reading failed \n\r"); /* Self explanatory */
    }
  }
}

void AD7746_breakout(void)
{

/* Relatively complete interface 
 * Set up assorted registers to get the modes we want
 * read out all the registers to verify correctness,
 * then take an infinite sequence of readings
 * reporting "raw" values to the terminal
 */
  unsigned char *ptr;
  int v=0;
  unsigned char bs=0;
  int i=0;
  int p=0;
  unsigned int c = 0;
  unsigned char data[20];
  for(v=0;v<20;v++)
  {
    data[v]=0;
  }
  v=0;

  /* Set addy to config reg */
  data[0]=0x0A;
  /* Write data = calibration mode, slow sampling */
  data[1]=0x01 | (7 << 3);
  
  PRINTF0("Performing first write\n\r");
  i2c0_master_send(0x90,data,2);
  
  /* Set addy to cap setup reg */
  data[0]=0x07;
  /* Write data = 0x80 = cap enabled */
  data[1]=0x80;
  i2c0_master_send(0x90,data,2);
  
  /* Set addy to EXC reg */
  data[0]=0x09;
  /* Set EXC source A */
  data[1]=0x08;
  i2c0_master_send(0x90,data,2);

  /*   Set addy to CAPDAC A reg */
  data[0]=0x0B;
  /* Set capdac a enabled and offset by about 1.2pF*/
  data[1]=0x00 | 39;
  i2c0_master_send(0x90,data,2);

  /* Set addy to config reg */
  data[0]=0x0A;
  /* Write data = 0x01 = continuous mode */
  data[1]=0x01 | (7 << 3);
  i2c0_master_send(0x90,data,2);
  
  /* Now hopefully configured */
  PRINTF0("Configuration steps complete\n\r");

  data[0]=0;
  data[1]=0;
  data[2]=0;
  
  i2c0_master_send(0x90,data,1);

  data[0]=0;
  data[1]=0;
  data[2]=0;

  /* Read all registers */
  PRINTF0("Reading all regs\r\n");
  for(i=0;i<18;i++)
  {
    data[0]=i;
    i2c0_master_send_receive(0x90,data,1,1);
    /* And print each register value out in hex */
    PRINTF2("%d: %X\n\r",i,data[0]);
    delay_ms(500);
  }

  /* OK-- now enter a loop */
  while(1)
  { 
    /* First read the status reg */
    data[0]=0x00;
    i2c0_master_send_receive(0x90,data,1,1);
    /* IF we've got a valid cap reading */
    if((data[0] & 0x01)==0)
    {
     
      /* read the data and report it */
      data[0]=0x01;
      i2c0_master_send_receive(0x90,data,1,3);
      PRINTF1("Measured %d\n\r",((data[0]<<16)+(data[1]<<8)+data[2]));
      delay_ms(1000);
    }
  }

}

