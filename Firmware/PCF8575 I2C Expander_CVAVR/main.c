/*****************************************************************************
*
* Atmel Corporation
*
* File              : main.c
* Compiler          : IAR EWAAVR 2.28a/3.10c
* Revision          : $Revision: 1.13 $
* Date              : $Date: 24. mai 2004 11:31:20 $
* Updated by        : $Author: ltwa $
*
* Support mail      : avr@atmel.com
*
* Supported devices : All devices with a TWI module can be used.
*                     The example is written for the ATmega16
*
* AppNote           : AVR315 - TWI Master Implementation
*
* Description       : Example of how to use the driver for TWI master 
*                     communication.
*                     This code reads PORTD and sends the status to the TWI slave.
*                     Then it reads data from the slave and puts the data on PORTB.
*                     To run the example use STK500 and connect PORTB to the LEDS,
*                     and PORTD to the switches. .
*
*                     This code has been modified to compile under a CodeVision AVR C complier
*                     It will now work without the STK500 and does not required pushbutton closures to operate.
*                     The TWI_Master.h has been modified:
*                     #define TWI_BUFFER_SIZE 25  was increased for the PCF8574 data 
*                     #define TWI_TWBR  set for 100khz with a 8Mhz clock. 
*                     This crazy struct and union thing was removed and replaced with a simple
*                     unsigned char flags for the lastTransOK and all status flags..
*                     was:
*                     struct TWI_statusReg lastTransOK
*                     union TWI_statusReg                       // Status byte holding flags.
*                     unsigned char all;
*                     unsigned char lastTransOK:1;      
*                     unsigned char unusedBits:7;  
*                     now:
*                     unsigned char TWI_statusReg_lastTransOK; 
*                     unsigned char TWI_statusReg_all;
    
****************************************************************************/
#include <mega16.h> 
#include <io.h>
#include <string.h>	
#include <delay.h>
#include <stdlib.h> 
#include <stdio.h> 
  
#include "TWI_Master.h"

#define TWI_GEN_CALL         0x20  // The General Call address is 0

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ  0x20

// Sample TWI transmission states, used in the main application.
#define SEND_DATA             0x01
#define REQUEST_DATA          0x02
#define READ_DATA_FROM_BUFFER 0x03


//TWCR    _SFR_IO8(0x36)
#define TWIE    0
#define TWEN    2
#define TWWC    3
#define TWSTO   4
#define TWSTA   5
#define TWEA    6
#define TWINT   7

static unsigned char TWI_buf[ TWI_BUFFER_SIZE ];    // Transceiver buffer
static unsigned char TWI_msgSize;                   // Number of bytes to be transmitted.
static unsigned char TWI_state = TWI_NO_STATE;      // State byte. Default set to TWI_NO_STATE.

//union TWI_statusReg TWI_statusReg = {0};            // TWI_statusReg is defined in TWI_Master.h

unsigned char TWI_Act_On_Failure_In_Last_Transmission ( unsigned char TWIerrorMsg )
{
                    // A failure has occurred, use TWIerrorMsg to determine the nature of the failure
                    // and take appropriate actions.
                    // Se header file for a list of possible failures messages.
                    
                    // Here is a simple sample, where if received a NACK on the slave address,
                    // then a retransmission will be initiated.
 
  if ( (TWIerrorMsg == TWI_MTX_ADR_NACK) | (TWIerrorMsg == TWI_MRX_ADR_NACK) )
    TWI_Start_Transceiver();
    
  return TWIerrorMsg; 
}



/****************************************************************************
Call this function to set up the TWI master to its initial standby state.
Remember to enable interrupts from the main application after initializing the TWI.
****************************************************************************/
void TWI_Master_Initialise(void)
{
  TWBR = TWI_TWBR;                                  // Set bit rate register (Baudrate). Defined in header file.
// TWSR = TWI_TWPS;                                  // Not used. Driver presumes prescaler to be 00.
  TWDR = 0xFF;                                      // Default content = SDA released.
  TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins.
         (0<<TWIE)|(0<<TWINT)|                      // Disable Interupt.
         (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests.
         (0<<TWWC);                                 //
}    
    
/****************************************************************************
Call this function to test if the TWI_ISR is busy transmitting.
****************************************************************************/
unsigned char TWI_Transceiver_Busy( void )
{
  return ( TWCR & (1<<TWIE) );                  // IF TWI Interrupt is enabled then the Transceiver is busy
}

/****************************************************************************
Call this function to fetch the state information of the previous operation. The function will hold execution (loop)
until the TWI_ISR has completed with the previous operation. If there was an error, then the function 
will return the TWI State code. 
****************************************************************************/
unsigned char TWI_Get_State_Info( void )
{
  while ( TWI_Transceiver_Busy() );             // Wait until TWI has completed the transmission.
  return ( TWI_state );                         // Return error state.
}

/****************************************************************************
Call this function to send a prepared message. The first byte must contain the slave address and the
read/write bit. Consecutive bytes contain the data to be sent, or empty locations for data to be read
from the slave. Also include how many bytes that should be sent/read including the address byte.
The function will hold execution (loop) until the TWI_ISR has completed with the previous operation,
then initialize the next operation and return.
****************************************************************************/
void TWI_Start_Transceiver_With_Data( unsigned char *msg, unsigned char msgSize )
{
  unsigned char temp;

  while ( TWI_Transceiver_Busy() );             // Wait until TWI is ready for next transmission.

  TWI_msgSize = msgSize;                        // Number of data to transmit.
  TWI_buf[0]  = msg[0];                         // Store slave address with R/W setting.
  if (!( msg[0] & (TRUE<<TWI_READ_BIT) ))       // If it is a write operation, then also copy data.
  {
    for ( temp = 1; temp < msgSize; temp++ )
      TWI_buf[ temp ] = msg[ temp ];
  }
  TWI_statusReg_all = 0;      
  TWI_state         = TWI_NO_STATE ;
  TWCR = (1<<TWEN)|                             // TWI Interface enabled.
         (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
         (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
         (0<<TWWC);                             //
}

/****************************************************************************
Call this function to resend the last message. The driver will reuse the data previously put in the transceiver buffers.
The function will hold execution (loop) until the TWI_ISR has completed with the previous operation,
then initialize the next operation and return.
****************************************************************************/
void TWI_Start_Transceiver( void )
{
  while ( TWI_Transceiver_Busy() );             // Wait until TWI is ready for next transmission.
  TWI_statusReg_all = 0;      
  TWI_state         = TWI_NO_STATE ;
  TWCR = (1<<TWEN)|                             // TWI Interface enabled.
         (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
         (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
         (0<<TWWC);                             //
}

/****************************************************************************
Call this function to read out the requested data from the TWI transceiver buffer. I.e. first call
TWI_Start_Transceiver to send a request for data to the slave. Then Run this function to collect the
data when they have arrived. Include a pointer to where to place the data and the number of bytes
requested (including the address field) in the function call. The function will hold execution (loop)
until the TWI_ISR has completed with the previous operation, before reading out the data and returning.
If there was an error in the previous transmission the function will return the TWI error code.
****************************************************************************/
unsigned char TWI_Get_Data_From_Transceiver( unsigned char *msg, unsigned char msgSize )
{
  unsigned char i;

  while ( TWI_Transceiver_Busy() );             // Wait until TWI is ready for next transmission.

  if( TWI_statusReg_lastTransOK )               // Last transmission competed successfully.              
  {                                             
    for ( i=0; i<msgSize; i++ )                 // Copy data from Transceiver buffer.
    {
      msg[ i ] = TWI_buf[ i ];
    }
  }
  return( TWI_statusReg_lastTransOK );                                   
}


// ********** Interrupt Handlers ********** //
/****************************************************************************
This function is the Interrupt Service Routine (ISR), and called when the TWI interrupt is triggered;
that is whenever a TWI event has occurred. This function should not be called directly from the main
application.
****************************************************************************/
//#pragma vector=TWI_vect
//interrupt void TWI_ISR(void)
interrupt [TWI] void TWI_ISR(void)  
 
{
  static unsigned char TWI_bufPtr ;
  
  switch (TWSR)
  {
    case TWI_START:             // START has been transmitted  
    case TWI_REP_START:         // Repeated START has been transmitted
      TWI_bufPtr = 0;                                     // Set buffer pointer to the TWI Address location
    case TWI_MTX_ADR_ACK:       // SLA+W has been tramsmitted and ACK received
    case TWI_MTX_DATA_ACK:      // Data byte has been tramsmitted and ACK received
      if (TWI_bufPtr < TWI_msgSize)
      {
        TWDR = TWI_buf[TWI_bufPtr++];
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
               (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           //
               (0<<TWWC);                                 //  
      }else                    // Send STOP after last byte
      {
        TWI_statusReg_lastTransOK = TRUE;                 // Set status bits to completed successfully. 
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (0<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
               (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // Initiate a STOP condition.
               (0<<TWWC);                                 //
      }
      break;
    case TWI_MRX_DATA_ACK:      // Data byte has been received and ACK tramsmitted
      TWI_buf[TWI_bufPtr++] = TWDR;
    case TWI_MRX_ADR_ACK:       // SLA+R has been tramsmitted and ACK received
      if (TWI_bufPtr < (TWI_msgSize-1) )                  // Detect the last byte to NACK it.
      {
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to read next byte
               (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after reception
               (0<<TWWC);                                 //  
      }else                    // Send NACK after next reception
      {
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to read next byte
               (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send NACK after reception
               (0<<TWWC);                                 // 
      }    
      break; 
    case TWI_MRX_DATA_NACK:     // Data byte has been received and NACK tramsmitted
      TWI_buf[TWI_bufPtr] = TWDR;
      TWI_statusReg_lastTransOK = TRUE;                 // Set status bits to completed successfully. 
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (0<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
             (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // Initiate a STOP condition.
             (0<<TWWC);                                 //
      break;      
    case TWI_ARB_LOST:          // Arbitration lost
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag
             (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|           // Initiate a (RE)START condition.
             (0<<TWWC);                                 //
      break;
    case TWI_MTX_ADR_NACK:      // SLA+W has been tramsmitted and NACK received
    case TWI_MRX_ADR_NACK:      // SLA+R has been tramsmitted and NACK received    
    case TWI_MTX_DATA_NACK:     // Data byte has been tramsmitted and NACK received
//    case TWI_NO_STATE              // No relevant state information available; TWINT = “0”
    case TWI_BUS_ERROR:         // Bus error due to an illegal START or STOP condition
    default:     
      TWI_state = TWSR;                                 // Store TWSR and automatically sets clears noErrors bit.
                                                        // Reset TWI Interface
      TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
             (0<<TWIE)|(0<<TWINT)|                      // Disable Interupt
             (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests
             (0<<TWWC);                                 //
  }
}

void main( void )
{
  unsigned char messageBuf[4],firsttime =1;
  unsigned char TWI_targetSlaveAddress,  TWI_operation=0,
                pressedButton =0, myCounter=0;

  //LED feedback port - connect port B to the STK500 LEDS
  DDRB  = 0x00;
  //PORTB = myCounter;
  
  //Switch port - connect portD to the STK500 switches
  DDRD  = 0x00;

  TWI_Master_Initialise();
 #asm ("sei") // global interrupt enable
  firsttime =1;
  
   
  TWI_targetSlaveAddress   = 0x20;   // A2,A1,A0 = 0,0,0 20H on PCF8574

  // This example is made to work together with the AVR311 TWI Slave application note and stk500.
  // In adition to connecting the TWI pins, also connect PORTB to the LEDS and PORTD to the switches.
  // The code reads the pins to trigger the action you request. There is an example sending a general call,
  // address call with Master Read and Master Write. The first byte in the transmission is used to send
  // commands to the TWI slave.

  // This is a stk500 demo example. The buttons on PORTD are used to control different TWI operations.
 for(;;)
  {  
   /* if (firsttime == 1)
      {
      TWI_Master_Initialise();
      messageBuf[1] = 0xfF;
      firsttime =2; 
      }
     if (firsttime == 2)
      {
      TWI_Master_Initialise();
      messageBuf[1] = 0xfE;
      firsttime =1; 
      }  */
      
    delay_ms(100); // Change this delay to get a different speed for the led flasher. 
     
     // if (pressedButton)       // Check if any button is pressed
    //{
      //do{temp = 1}      // Wait until key released    
      
      //while (1);
      //pressedButton =0 ;
      switch ( pressedButton ) 
      {
        // Modification to Atmel test code to work on a Mega16 without a STK500 kit. 
        // Send a Address Call, sending a command and data to the Slave    
          case 0:      
          messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT); // The first byte must always consit of General Call code or the TWI slave address.
                                                        //  OxFE turns on led , OxFF turns off led 
          messageBuf[1] = 0xfF;                        // The second byte is used for the data. Turn on 0 led
          messageBuf[2] = 0xFF;                        // The third byte is used for the data. 8-16 all high 
          TWI_Start_Transceiver_With_Data( messageBuf, 3 ); 
           pressedButton = 2;
          break;
       
       
       
        case 1:      
          messageBuf[0] = TWI_GEN_CALL;     // The first byte must always consit of General Call code or the TWI slave address.
          messageBuf[1] = 0x10;             // The command or data to be included in the general call.
          TWI_Start_Transceiver_With_Data( messageBuf, 2 );
          break;

        // Send a Address Call, sending a command and data to the Slave          
        case 2:      
          messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT); // The first byte must always consit of General Call code or the TWI slave address.
                                                        //  OxFE turns on led , OxFF turns off led 
          messageBuf[1] = 0xfe;                        // The second byte is used for the data. Turn on 0 led
          messageBuf[2] = 0xFF;                        // The third byte is used for the data. 8-16 all high 
          TWI_Start_Transceiver_With_Data( messageBuf, 3 );
           pressedButton = 0;
          break;

        // Send a Address Call, sending a request, followed by a receive          
        case 3:      
          // Send the request-for-data command to the Slave
          messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT); // The first byte must always consit of General Call code or the TWI slave address.
          messageBuf[1] = TWI_CMD_MASTER_READ;             // The first byte is used for commands.
          TWI_Start_Transceiver_With_Data( messageBuf, 2 );

          TWI_operation = REQUEST_DATA;         // To release resources to other operations while waiting for the TWI to complete,
                                                // we set a operation mode and continue this command sequence in a "followup" 
                                                // section further down in the code.
                    
        // Get status from Transceiver and put it on PORTB
        case 4:
          PORTB = TWI_Get_State_Info();
          break;

        // Increment myCounter and put it on PORTB          
        case 5:      
          PORTB = ++myCounter;        
          break;
          
        // Reset myCounter and put it on PORTB
        case 6:      
          PORTB = myCounter = 0;        
          break;  
     // }
    }    

    if ( ! TWI_Transceiver_Busy() )
    {
    // Check if the last operation was successful
      if ( TWI_statusReg_lastTransOK )
      {
        if ( TWI_operation ) // Section for follow-up operations.
        {
        // Determine what action to take now
          if (TWI_operation == REQUEST_DATA)
          { // Request/collect the data from the Slave
            messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (TRUE<<TWI_READ_BIT); // The first byte must always consit of General Call code or the TWI slave address.
            TWI_Start_Transceiver_With_Data( messageBuf, 2 );       
            TWI_operation = READ_DATA_FROM_BUFFER; // Set next operation        
          }
          else 
          if (TWI_operation == READ_DATA_FROM_BUFFER)
          { // Get the received data from the transceiver buffer
            TWI_Get_Data_From_Transceiver( messageBuf, 2 );
            PORTB = messageBuf[1];        // Store data on PORTB.
            TWI_operation = FALSE;        // Set next operation        
          }
        }
      }
      else // Got an error during the last transmission
      {
        // Use TWI status information to detemine cause of failure and take appropriate actions. 
        TWI_Act_On_Failure_In_Last_Transmission( TWI_Get_State_Info( ) );
      }
    }

    // Do something else while waiting for TWI operation to complete and/or a switch to be pressed
    //__no_operation(); // Put own code here.

  }
}


/*
  // This example code runs forever; sends a byte to the slave, then requests a byte
  // from the slave and stores it on PORTB, and starts over again. Since it is interupt
  // driven one can do other operations while waiting for the transceiver to complete.
  
  // Send initial data to slave
  messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
  messageBuf[1] = 0x00;
  TWI_Start_Transceiver_With_Data( messageBuf, 2 );

  TWI_operation = REQUEST_DATA; // Set the next operation

  for (;;)
  {
    // Check if the TWI Transceiver has completed an operation.
    if ( ! TWI_Transceiver_Busy() )                              
    {
    // Check if the last operation was successful
      if ( TWI_statusReg_lastTransOK )
      {
      // Determine what action to take now
        if (TWI_operation == SEND_DATA)
        { // Send data to slave
          messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
          TWI_Start_Transceiver_With_Data( messageBuf, 2 );
          
          TWI_operation = REQUEST_DATA; // Set next operation
        }
        else if (TWI_operation == REQUEST_DATA)
        { // Request data from slave
          messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (TRUE<<TWI_READ_BIT);
          TWI_Start_Transceiver_With_Data( messageBuf, 2 );
          
          TWI_operation = READ_DATA_FROM_BUFFER; // Set next operation        
        }
        else if (TWI_operation == READ_DATA_FROM_BUFFER)
        { // Get the received data from the transceiver buffer
          TWI_Get_Data_From_Transceiver( messageBuf, 2 );
          PORTB = messageBuf[1];        // Store data on PORTB.
          
          TWI_operation = SEND_DATA;    // Set next operation        
        }
      }
      else // Got an error during the last transmission
      {
        // Use TWI status information to detemine cause of failure and take appropriate actions. 
        TWI_Act_On_Failure_In_Last_Transmission( TWI_Get_State_Info( ) );
      }
    }
    // Do something else while waiting for the TWI Transceiver to complete the current operation
    __no_operation(); // Put own code here.
  }
}
*/
