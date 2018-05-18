/******************************************************************************/
/**
 * \section CAN MODULE
 *
 *
 */
/******************************************************************************/

/******************************************************************************/
/**
 * \file 
 *
 * \brief Library offering CAN module functions
 *
 * \version 1.0 (original)
 * \version 1.1 (PICEBS 2 release)
 *
 * \date 2015-04-30
 *
 * \author sap@hevs.ch
 *
 */
/******************************************************************************/
#ifndef CAN_H_
#define CAN_H_
#include <pic18.h>
#include <stdint.h>


// constants of CAN system

#define CAN_OK         (0)
#define CAN_FAILTX     (2)
#define CAN_NOMSG      (4)
#define CAN_FAIL       (0xff)

#define CAN_MAX_CHAR_IN_MESSAGE (8)

#define CAN_NO_INT	0
#define CAN_RX_INT	1
#define CAN_TX_INT	2
#define CAN_ERR_INT	3
//------------------------------------------------------------------------------
#define SIZE_OF_RX_BUFFER 4   ///< size of receive fifo buffer
#define SIZE_OF_TX_BUFFER 4   ///< size of transmit fifo buffer

//------------------------------------------------------------------------------
// pin selection depending on board
//------------------------------------------------------------------------------
#define nCAN_RST_DIR          TRISF6
#define nCAN_RST              LATF6
#define nCAN_POWERDOWN_DIR    TRISF7
#define nCAN_POWERDOWN        LATF7
#define CAN_INTE              INT3IE
#define CAN_INTEDGE           INTEDG3
#define CAN_INTF              INT3IF
#define CAN_INTPIN            RB3

/******************************************************************************/
/**
 * \brief CAN speed structure 
 Used for custom speed control. See MCP2515 datasheet for user selection.
*/
/******************************************************************************/
struct CANSPEED
{
  uint8_t   brp;	///< baudrate prescaler max:63
  uint8_t   sjw;	///< sjw control : 0-3 (resynchronisation)
  uint8_t   prseg;	///< propagation time : 0-7
  uint8_t   phseg1;	///< segment phase 1 : 0-7
  uint8_t   phseg2;	///< segment phase 2 : 0-7
  uint8_t   sam;	///< sample points bit(0: one sample / 1: three samples)
  uint8_t   btlmode;	///< blt bit mode : 0 or 1 (control phase2 segment)
};

/******************************************************************************/
/**
 * \brief CAN system filters
 
 Used for filters control.
*/
/******************************************************************************/
struct CANFILTER
{
  uint32_t  mask0;	///< mask 0 (for filter0 and filter1)
  uint32_t  mask1;	///< mask 1 (for filters 2 to 5)
  uint32_t  filter0;	///< filter 0 (with mask 0)
  uint32_t  filter1;	///< filter 1 (with mask 0)
  uint32_t  filter2;	///< filter 2 (with mask 1)
  uint32_t  filter3;	///< filter 3 (with mask 1)
  uint32_t  filter4;	///< filter 4 (with mask 1)
  uint32_t  filter5;	///< filter 5 (with mask 1)
  uint8_t   ext;	///< extended identifiers (0=no:11 bits / 1=yes:29 bits)
};

/******************************************************************************/
/**
 * \brief CAN message structure
 
 Used for any message to send or read from CAN bus.
*/
/******************************************************************************/
struct CANMESSAGE
{
  uint8_t   extended_identifier;    ///< identifier CAN is extended (1 = yes)
  uint32_t  identifier;             ///< either extended or standard identifier
  uint8_t   dlc;                    ///< data length of message : 0-8
  uint8_t   dta[CAN_MAX_CHAR_IN_MESSAGE];   ///< message up to 8 bytes
  uint8_t   rtr;                            ///< 1=remote transfer request
  uint8_t   filhit;		///< Accept. Filter that enabled the reception
  uint8_t   txPrio; ///< Send priority (0 to 3, 3 is max)
};

/******************************************************************************/
/*!
 * \defgroup CANGrp CAN module functions 
 * \ref can.h
 
 */
/******************************************************************************/

#define MISO_DIR  TRISC4
#define MOSI_DIR  TRISC5
#define SCK_DIR   TRISC3
#define CS_DIR    TRISD1
#define SPI_CS    LATD1

/******************************************************************************/
extern const struct CANSPEED 	canSpeed10k;
extern const struct CANSPEED 	canSpeed20k;
extern const struct CANSPEED 	canSpeed50k;
extern const struct CANSPEED 	canSpeed100k;
extern const struct CANSPEED 	canSpeed125k;
extern const struct CANSPEED 	canSpeed250k;
extern const struct CANSPEED 	canSpeed500k;
extern const struct CANSPEED 	canSpeed800k;
extern const struct CANSPEED 	canSpeed1000k;


void Spi_ReadWrite(uint8_t * rx_tx, uint32_t size);
uint8_t Spi_Read(void);
void Spi_Write(uint8_t * tx, uint32_t size);
//@{ 
/******************************************************************************/
/**
 * \brief  Inits the CAN interface
 * \param speedCfg Configuration for the selected CAN speed
  \li   canSpeed10k
  \li   canSpeed20k
  \li   canSpeed50k
  \li   canSpeed100k
  \li   canSpeed125k
  \li   canSpeed250k
  \li   canSpeed500k
  \li   canSpeed800k
  \li   canSpeed1000k
 * \param filterCfg Configuration for filters of CAN interface
 * \param interruptServiceRoutine Interrupt service routine
 \li   Set interruptServiceRoutine to 0 if no interrupt service routine is used
 \li If the user wants to use an Isr, see the example below
\code
{
  ...
	selFilters.mask0 = 0x7F0;		// all bits are filtered (filter0 & filter1)
	selFilters.mask1 = 0x7FF;		// all bits are filtered (filter2 to filter5)
	selFilters.filter0 = 0x7FF;	// filter address 0x7FF;
	selFilters.filter1 = 0x1;		// filter address 0x01;
	selFilters.filter2 = 0xAA;	// filter address 0xAA;
	selFilters.filter3 = 0x00;	// filter address 0x00;
	selFilters.filter4 = 0x00;	// filter address 0x00;
	selFilters.filter5 = 0x00;	// filter address 0x00;
	selFilters.ext = 0;					// standard identifiers used;	
  Can_Init(canSpeed125k,&selFilters); // example of call for 125K
  ...
}

void interrupt AnyIsr(void)
{

  if((CAN_INTF == 1)&&(CAN_INTE == 1)) // interrupt flag & active
  {
    CAN_INTF = 0;               // clear interrupt
    Can_Isr();                  // interrupt treatment
    if(CAN_INTPIN == 0)         // check pin is high again
    {
      CAN_INTF = 1;             // no -> re-create interrupt
    }
  }
}

\endcode	
* The Isr will be called when any RX buffer will become full, when any TX buffer
 * becomes empty and when \ref CanPutMessage is called
 * \sa The Function needs to be called once before any other CAN functions.
*/
/******************************************************************************/
void Can_Init(const struct CANSPEED * speedCfg,struct CANFILTER * filterCfg);

/******************************************************************************/
/**
 * \brief  Power off the CAN interface
*  The driver is turned off and the MCP2515 is in sleep mode
*/
/******************************************************************************/
void Can_PowerOff(void);

/******************************************************************************/
/**
 * \brief  Send a message to the CAN bus
 * \param msg Message address to be sent to the CAN bus
 * \return Return code : always CAN_OK = 0

	
* This function waits until a TX buffer is ready to send data to the CAN bus.
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
static uint8_t Can_SendMessage(const struct CANMESSAGE* msg);

/******************************************************************************/
/**
 * \brief  Read a message from the CAN bus (without interrupt mode)
 * \param msg Message address to read from the CAN bus
 * \return Return code
  \li   CAN_OK : a message has been readed
  \li   CAN_NOMSG : there was no message to read
\code
{
  ...
  retCode = Can_ReadMessage(&msg);  // send message
	if(retCode != CAN_OK)       // return no message read
	{...}
  ...
}
\endcode						
* This function reads the RX buffer and get a message if there is one. If no message
* is on any RX buffer, the code CAN_NOMSG is returned.			
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
static uint8_t Can_ReadMessage(struct CANMESSAGE *msg);

/******************************************************************************/
/**
 * \brief  Read the interrupts status af the CAN interface
 * \return Status bits (---TTTRR) T is TX buffer, R is RX buffer
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
static uint8_t Can_GetStatus(void);

/******************************************************************************/
/**
 * \brief  Return the number of message in the FIFO receive buffer
 * \return Number of message
* This function indicates the number of messages in the FIFO
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
uint8_t Can_InBufferStatus(void);

/******************************************************************************/
/**
 * \brief  Return the number of message in the FIFO transmit buffer
 * \return Number of message
* This function indicates the number of messages in the FIFO
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
uint8_t Can_OutBufferStatus(void);

/******************************************************************************/
/**
 * \brief  Read a message from the FIFO (interrupt mode)
 * \param msg Message address to copy from the FIFO
* This function copy the Fifo data to the given structure
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
uint8_t Can_GetMessage(struct CANMESSAGE * msgPtr);
/******************************************************************************/
/**
 * \brief  Write a message to the FIFO
 * \param msg Message address to copy to the FIFO
\code
{
  ...
	msg.extended_identifier = 0;    // identifier is standard
	msg.identifier = 0xAA;		// identifier is 0xAA
	msg.dlc = 2;			// there is two data bytes
	msg.dta[0] = 'A';		// first data byte is 'A'
	msg.dta[1] = 'F';		// second data byte is 'F'
	msg.rtr = 0;			// no remote transmit required
  Can_PutMessage(&msg); 		// send message
  ...
}
\endcode
 *  * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
uint8_t Can_PutMessage(struct CANMESSAGE * msgPtr);

/******************************************************************************/
/**
 * \brief  Read a message from the CAN bus and copy it in FIFO buffer
 * \return Return code
  \li   0 : the message has been copied
  \li   1 : no message read, FIFO buffer is full
* This function reads the RX buffer and get a message if there is one.
 * This function is called by \ref Can_Isr
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
static uint8_t Can_ReadInBuffer(void);

/******************************************************************************/
/**
 * \brief  Writes a message from the FIFO to the CAN bus
 * \return Return code
  \li   0 : the message has been sent
  \li   1 : no message in FIFO buffer
* This function reads the TX buffer and sent a message if there is one.
 * This function is called by \ref Can_Isr
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
static uint8_t Can_WriteOutBuffer(void);

/******************************************************************************/
/**
 * \brief  Isr to be called on interrupt
 * \code

void interrupt AnyIsr(void)
{

  if((CAN_INTF == 1)&&(CAN_INTE == 1)) // interrupt flag & active
  {
    CAN_INTF = 0;               // clear interrupt
    Can_Isr();                  // interrupt treatment
    if(CAN_INTPIN == 0)         // check pin is high again
    {
      CAN_INTF = 1;             // no -> re-create interrupt
    }
  }
}

\endcode	
* This function fills the recevie fifo buffer
 * \sa The Function \ref Can_Init has to be called once before.
*/
/******************************************************************************/
void Can_Isr(void);

//@}
#endif
