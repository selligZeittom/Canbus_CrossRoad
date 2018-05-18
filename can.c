/******************************************************************************/
/* FILENAME		: CAN.C                                                         */
/*----------------------------------------------------------------------------*/
/* VERSION		: 0.1                                                           */
/*----------------------------------------------------------------------------*/
/* REVISION		: - 0.1 11/2009 (initial revision)                              */
/******************************************************************************/
#include <pic18.h>
#include <string.h>
#include <stdlib.h>
#include "can.h"
#include "mcp2515.h"
/*----------------------------------------------------------------------------*/
/* STANDATD DEFINED SPEEDS FOR CAN : BRP ,JSW, PRSEG,SEG1,SEG2, SAM,BLT       */
/* These parameters are fixed for the 16MHz oscillator                        */
/*----------------------------------------------------------------------------*/
const struct CANSPEED 	canSpeed10k 	= {31 ,1, 7,7,7, 1,1};
const struct CANSPEED 	canSpeed20k 	= {19 ,1, 6,6,4, 1,1};
const struct CANSPEED 	canSpeed50k 	= {15 ,1, 2,3,1, 1,1};
const struct CANSPEED 	canSpeed100k  = {7  ,1, 2,3,1, 1,1};
const struct CANSPEED 	canSpeed125k  = {7  ,1, 0,3,1, 1,1};
const struct CANSPEED 	canSpeed250k 	= {3  ,1, 0,3,1, 1,1};
const struct CANSPEED 	canSpeed500k  = {1  ,1, 0,3,1, 1,1};
const struct CANSPEED 	canSpeed800k  = {0  ,1, 2,3,1, 1,1};
const struct CANSPEED 	canSpeed1000k = {0  ,1, 0,3,1, 1,1};

/*----------------------------------------------------------------------------*/
/* Receive data buffer with pointers to control this FIFO                     */
/*----------------------------------------------------------------------------*/
struct CANMESSAGE rxBuffer[SIZE_OF_RX_BUFFER];
uint8_t rxBufferPtrIn;
uint8_t rxBufferPtrOut;
uint8_t rxBufferCount;

struct CANMESSAGE txBuffer[SIZE_OF_TX_BUFFER];
uint8_t txBufferPtrIn;
uint8_t txBufferPtrOut;
uint8_t txBufferCount;

volatile uint8_t nbTx0 = 0;
volatile uint8_t nbTx1 = 0;
volatile uint8_t nbTx2 = 0;

/******************************************************************************/
/* FUNCTION		: Can_Isr                                                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Called on interrupt to get incoming messages                    */
/******************************************************************************/
void Can_Isr(void)
{
  static uint8_t nbTxBuf=3;   // there are 3 HW tx buffers
  uint8_t retCode;

  retCode = Can_GetStatus();  // get all and clear TX interrupts
  //----------------------------------------------------------------------------
  if((retCode & 0x03) != 0)   // any receive buffer
  {
	Can_ReadInBuffer();       // copy messages in receive fifo
  }
  //----------------------------------------------------------------------------
  if((retCode & 0x1C) != 0)   // any tx buffer ready
  {
    if((retCode & 0x10) != 0) // if tx buffer empty, increment tx counter
    {
      nbTxBuf++;
    }
    if((retCode & 0x08) != 0) // if tx buffer empty, increment tx counter
    {
      nbTxBuf++;
    }
    if((retCode & 0x04) != 0) // if tx buffer empty, increment tx counter
    {
      nbTxBuf++;
    }
  }
  //----------------------------------------------------------------------------
  while((Can_OutBufferStatus() != 0) && (nbTxBuf != 0))  // someting to send ?
  {
    Can_WriteOutBuffer();     // write to tx HW buffer
    nbTxBuf--;                // decrement counter
  }

}

/******************************************************************************/
/* FUNCTION		: Can_ReadInBuffer   																						*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: -                                                               */
/* OUTPUTS		: 0 if OK, 1 if buffer full         														*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Used by ISR to copy data from HW buffer to SW buffers           */
/******************************************************************************/
static uint8_t Can_ReadInBuffer(void)
{
  uint8_t i;
  uint8_t error=0;
  uint8_t rdStatus;
  
  for(i=0;i<2;i++)        // reads the two input buffers
  {
    rdStatus = Can_ReadMessage(&rxBuffer[rxBufferPtrIn]);
    if((rxBufferCount) < SIZE_OF_RX_BUFFER)
    {
      if(rdStatus == CAN_OK)
      {
        rxBufferCount++;
        rxBufferPtrIn = (rxBufferPtrIn + 1) % SIZE_OF_RX_BUFFER;
      }
    }
    else
    {
      error = 1;
    }
  }
  return error;
}

/******************************************************************************/
/* FUNCTION		: Can_WriteOutBuffer 																						*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: -                                                               */
/* OUTPUTS		: 0 if OK, 1 if buffer full         														*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Used by ISR to copy data from SW buffer to HW buffers           */
/******************************************************************************/
static uint8_t Can_WriteOutBuffer(void)
{
  Can_SendMessage(&txBuffer[txBufferPtrOut]);
  txBufferPtrOut = (txBufferPtrOut + 1) % SIZE_OF_TX_BUFFER;
  txBufferCount--;
  return 0;
}

/******************************************************************************/
/* FUNCTION		: Can_InBufferStatus 																						*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: -                                                               */
/* OUTPUTS		: Number of message in FIFO buffer															*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Used to know if a message is in the SW buffer                   */
/******************************************************************************/
uint8_t Can_InBufferStatus(void)
{
  return rxBufferCount;
}

/******************************************************************************/
/* FUNCTION		: Can_OutBufferStatus 																					*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: -                                                               */
/* OUTPUTS		: Number of message in FIFO buffer															*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Used to know if a message is in the SW buffer                   */
/******************************************************************************/
uint8_t Can_OutBufferStatus(void)
{
  return txBufferCount;
}

/******************************************************************************/
/* FUNCTION		: Can_GetMessage                  															*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: Pointer to message to copy in                                   */
/* OUTPUTS		: 0 - ok message, 1 - no m essages in buffer                    */
/*----------------------------------------------------------------------------*/
/* GOAL			: Used to read the SW FIFO buffer                                 */
/******************************************************************************/
uint8_t Can_GetMessage(struct CANMESSAGE * msgPtr)
{
  uint8_t i;
  if(rxBufferCount > 0)
  {
    msgPtr->dlc = rxBuffer[rxBufferPtrOut].dlc;
    msgPtr->extended_identifier = rxBuffer[rxBufferPtrOut].extended_identifier;
    msgPtr->filhit = rxBuffer[rxBufferPtrOut].filhit;
    msgPtr->identifier = rxBuffer[rxBufferPtrOut].identifier;
    msgPtr->rtr = rxBuffer[rxBufferPtrOut].rtr;
    for(i=0;i< msgPtr->dlc;i++)
    {
      msgPtr->dta[i] = rxBuffer[rxBufferPtrOut].dta[i];
    }
    CAN_INTE = 0;
    rxBufferPtrOut = (rxBufferPtrOut + 1) % SIZE_OF_RX_BUFFER;
    rxBufferCount--;
    CAN_INTE = 1;
    return 0;
  }
  return 1;
}

/******************************************************************************/
/* FUNCTION		: Can_PutMessage                  															*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: Pointer to message to copy in                                   */
/* OUTPUTS		: return 0 if OK, return 1 on buffer full                       */
/*----------------------------------------------------------------------------*/
/* GOAL			: Used to write to the SW FIFO buffer (and activate ISR)          */
/******************************************************************************/
uint8_t Can_PutMessage(struct CANMESSAGE * msgPtr)
{
  uint8_t i;

  if((txBufferCount) < SIZE_OF_TX_BUFFER)
  {
    txBuffer[txBufferPtrIn].dlc =  msgPtr->dlc;
    txBuffer[txBufferPtrIn].extended_identifier = msgPtr->extended_identifier;
    txBuffer[txBufferPtrIn].filhit = msgPtr->filhit;
    txBuffer[txBufferPtrIn].identifier = msgPtr->identifier;
    txBuffer[txBufferPtrIn].rtr = msgPtr->rtr;
    txBuffer[txBufferPtrIn].txPrio = msgPtr->txPrio;
    for(i=0;i< msgPtr->dlc;i++)
    {
      txBuffer[txBufferPtrIn].dta[i] = msgPtr->dta[i];
    }
    CAN_INTE = 0;
    txBufferCount++;
    txBufferPtrIn = (txBufferPtrIn + 1) % SIZE_OF_TX_BUFFER;
    CAN_INTE = 1;
    CAN_INTF = 1;
    return 0;
  }
  else
  {
    return 1;
  }
}

/******************************************************************************/
/* FUNCTION		: CAN_Init                                    									*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: CANSPEED, CANFILTER																							*/
/* OUTPUTS		: -																															*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Init the CAN interface with the passed structures								*/
/******************************************************************************/
void Can_Init(const struct CANSPEED * speedCfg,
							struct CANFILTER * filterCfg)
{
  nCAN_RST_DIR = 0;
  nCAN_POWERDOWN_DIR = 0;
  nCAN_RST = 1;
  nCAN_POWERDOWN = 0;
  MISO_DIR = 1;
  MOSI_DIR = 0;
  SCK_DIR = 0;
  CS_DIR = 0;
  //----------------------------------------------------------------------------
  // SPI initialisation
  //----------------------------------------------------------------------------
  CKE1 = 0;                   // mode of data transfer
  CKP1 = 1;                   // used with CAN controller
  SSPCON1 = 0b00111010;       // SSPEN & CKP & speed select (fosc/8) 8MHz
  SSPEN1 = 1;                 // enable SPI
  //----------------------------------------------------------------------------
  // CAN chipset initialisation
  //----------------------------------------------------------------------------
	Mcp2515_Init(speedCfg,filterCfg);		// init CAN interface
    CAN_INTE = 1;
    CAN_INTEDGE = 0;
    
}

/******************************************************************************/
/* FUNCTION		: Can_PowerOff                                 									*/
/*----------------------------------------------------------------------------*/
/* INPUTS     : -                 																						*/
/* OUTPUTS		: -																															*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Reduce consumption                              								*/
/******************************************************************************/
void Can_PowerOff(void)
{
  Mcp2515Sleep();
  nCAN_POWERDOWN = 1;         // turn off CAN driver to reduce current
}

/******************************************************************************/
/* FUNCTION		: Spi_RW                                                        */
/*----------------------------------------------------------------------------*/
/* INPUTS		: UINT8 * tx -> data to send to SPI bus                           */
/* 				  UINT8 * rx -> data received from SPI bus                          */
/* 				  UINT32  size -> size to write (read)                              */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Read or write data to/from SPI bus                              */
/******************************************************************************/
static void SPIRW(const uint8_t *tx, uint8_t *rx, uint32_t size)
{
  uint32_t i;

  SSPCON1 = 0b00111010;     // SSPEN & CKP & speed select (fosc/8) 8MHz

  SPI_CS = 0;               // select CAN SPI
  for(i=0;i<size;i++)
  {
    SSPBUF = tx[i];         // send a data
    while(SSPIF == 0){}     // wait send finished
    SSP1IF = 0;             // clear flag
    if(rx != 0)
      rx[i] = SSPBUF;
  }
  SPI_CS = 1;               // unselect SPI
}

/******************************************************************************/
/* FUNCTION		: Spi_ReadWrite                                                 */
/*----------------------------------------------------------------------------*/
/* INPUTS		: UINT8 * data pointer to send/and receive to/from SPI bus        */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Write and read data to/from SPI bus                             */
/******************************************************************************/
void Spi_ReadWrite(uint8_t * rx_tx, uint32_t size)
{
	SPIRW(rx_tx, rx_tx, size);
}

/******************************************************************************/
/* FUNCTION		: Spi_Write                                                     */
/*----------------------------------------------------------------------------*/
/* INPUTS		: UINT8 * data pointer to send to SPI bus                         */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Write data to SPI bus                                           */
/******************************************************************************/
void Spi_Write(uint8_t * tx, uint32_t size)
{
	SPIRW(tx, 0, size);
}

/******************************************************************************/
/* FUNCTION		: Can_GetStatus																									*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: -                                                               */
/* OUTPUTS		: return Status																									*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Reads the status of the CAN interface														*/
/*				  - return CAN_NO_INT (no interrupt occured)												*/
/*				  - return CAN_RX_INT (a message is arrived)												*/
/*				  - return CAN_TX_INT (a TX buffer is empty)												*/
/*				  - return CAN_ERR_INT (an error occured)														*/
/******************************************************************************/
static uint8_t Can_GetStatus(void)
{
	uint8_t status;
	uint8_t msg[]={MCP_READ,MCP_CANINTF,0};		// a read command of 3 bytes
	uint8_t msg4[]={MCP_BITMOD,MCP_CANINTF,MCP_TX_INT,0};
	
  Spi_ReadWrite(msg,sizeof(msg));				// read RX buffer	
	status = msg[2];
	msg4[2] = status & MCP_TX_INT;
  Spi_ReadWrite(msg4,sizeof(msg4));				// read RX buffer
	return status;
}

/******************************************************************************/
/* FUNCTION		: Can_ReadMessage																								*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: struct CANMESSAGE																								*/
/* OUTPUTS		: return Code																										*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Check if a messsage has been received														*/
/*				  - return CAN_OK (message has been placed in struct)								*/
/*				  - return CAN_NOMSG (no message available)													*/
/******************************************************************************/
static uint8_t Can_ReadMessage(struct CANMESSAGE *msg)
{
	uint8_t buf[4];											// I/O buffer
	
	buf[0] = MCP_READ_STATUS;
	Spi_ReadWrite(buf,2); 						// Read status register
	/*--------------------------------------------------------------------------*/
	if ( buf[1] & MCP_STAT_RX0IF ) 		// there is a data in RX buffer 0
	{
		Mcp2515ReadCanMsg( MCP_RXBUF_0, msg);	// read message
		buf[0] = MCP_BITMOD;
		buf[1] = MCP_CANINTF;
		buf[2] = MCP_RX0IF;
		buf[3] = 0;
		Spi_Write(buf,4); 							// clear INTF bit
		buf[0] = MCP_BITMOD;
		buf[1] = MCP_CANINTE;
		buf[2] = MCP_RX0IF;
		buf[3] = 0xFF;
		Spi_Write(buf,4); 							// set INTE bit
		return CAN_OK;
	}
	/*--------------------------------------------------------------------------*/
	else if ( buf[1] & MCP_STAT_RX1IF ) // there is a data in RX buffer 1
	{
		// Msg in Buffer 1
		Mcp2515ReadCanMsg( MCP_RXBUF_1, msg);
		buf[0] = MCP_BITMOD;
		buf[1] = MCP_CANINTF;
		buf[2] = MCP_RX1IF;
		buf[3] = 0;
		Spi_Write(buf,4); 							// clear INTF bit
		buf[0] = MCP_BITMOD;
		buf[1] = MCP_CANINTE;
		buf[2] = MCP_RX1IF;
		buf[3] = 0xFF;
		Spi_Write(buf,4); 							// set INTE bit
		return CAN_OK;
	}
	/*--------------------------------------------------------------------------*/
	return CAN_NOMSG;
}


/******************************************************************************/
/* FUNCTION		: Can_SendMessage																								*/
/*----------------------------------------------------------------------------*/
/* INPUTS		: struct CANMESSAGE																								*/
/* OUTPUTS		: return Code																										*/
/*----------------------------------------------------------------------------*/
/* GOAL			: Check if a messsage has been received														*/
/*				  - return CAN_OK (message has been placed in struct)								*/
/*				  - return CAN_FAILTX (message not send)														*/
/******************************************************************************/
static uint8_t Can_SendMessage(const struct CANMESSAGE * msg)
{
	uint8_t res, txbuf_n;
	uint8_t buf[4];

	/*--------------------------------------------------------------------------*/
	do 															// wait until a TX buffer is free
	{
		res = Mcp2515GetNextFreeTXBuf(&txbuf_n); 	// wait rdy for transmit on
																							// any of three TX buffers
	} while (res == MCP_ALLTXBUSY);
	/*--------------------------------------------------------------------------*/	
	if (res != MCP_ALLTXBUSY) 				// normally always OK (time check ?)
	{
		Mcp2515WriteCanMsg( txbuf_n, msg);		// write message in CAN buffers
		buf[0] = MCP_WRITE;                   // bit write
		buf[1] = txbuf_n-1;										// on free TX buffer
		buf[2] = MCP_TXB_TXREQ_M | msg->txPrio;          // request to send
		Spi_Write(buf,3); 										// Send message
		return CAN_OK;
	}
	else 
	{
		return CAN_FAILTX;										// any error
	}
}

