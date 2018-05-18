/******************************************************************************/
/* FILENAME		: MCP2551.C                                                     */
/*----------------------------------------------------------------------------*/
/* VERSION		: 0.1                                                           */
/*----------------------------------------------------------------------------*/
/* REVISION		: - 0.1 11/2009 (initial revision)                              */
/******************************************************************************/
#include "can.h"
#include "mcp2515.h"
#include "mcp2515_defs.h"
#include <stddef.h>
#include <string.h>
/******************************************************************************/
/* FUNCTION		: Mcp2515_Reset                                                 */
/*----------------------------------------------------------------------------*/
/* INPUTS		: -                                                               */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Reset the MCP2551 (idem to tie /RESET pin low)                  */
/******************************************************************************/
void Mcp2515_Reset(void)
{
	uint32_t i;
	uint8_t	cmd = MCP_RESET;			// reset command
	
	Spi_Write(&cmd,sizeof(cmd));		// send a reset command
	for(i=0;i<10000;i++){}				// enough delay (min. 128 clk cycle)
}

/******************************************************************************/
/* FUNCTION		: Mcp2515_Init                                                  */
/*----------------------------------------------------------------------------*/
/* INPUTS		: CANSPEEED speedCfg, CANFILTER filterCfg                         */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Init the MCP2551 and configure speed & filters                  */
/******************************************************************************/
void Mcp2515_Init(const struct CANSPEED * speedCfg,struct CANFILTER * filterCfg)
{
	uint8_t	i;
	uint8_t cmdMask4[]={MCP_BITMOD,MCP_CANCTRL,MODE_MASK,MODE_CONFIG};
	uint8_t cmdWrite3[]={MCP_WRITE,0,0};

	Mcp2515_Reset();						// reset the chip
	/*--------------------------------------------------------------------------*/
	/* Set mode configuration													*/
	/*--------------------------------------------------------------------------*/
	Spi_Write(cmdMask4,sizeof(cmdMask4));	// mode configuration
	/*--------------------------------------------------------------------------*/
	/* Set speed configuration													*/
	/*--------------------------------------------------------------------------*/
	cmdWrite3[1] = MCP_CNF1;
	cmdWrite3[2] = speedCfg->brp | speedCfg->sjw << 6;
	Spi_Write(cmdWrite3,sizeof(cmdWrite3));
	/*--------------------------------------------------------------------------*/
	cmdWrite3[1] = MCP_CNF2;
	cmdWrite3[2] = speedCfg->prseg 
				| speedCfg->phseg1 << 3 
				| speedCfg->sam << 6
				| speedCfg->btlmode << 7;
	Spi_Write(cmdWrite3,sizeof(cmdWrite3));
	/*--------------------------------------------------------------------------*/
	cmdWrite3[1] = MCP_CNF3;
	cmdWrite3[2] = speedCfg->phseg2 | speedCfg->sjw << 6;
	Spi_Write(cmdWrite3,sizeof(cmdWrite3));
	/*--------------------------------------------------------------------------*/
	/* CAN buffers filters & mask initialisation 								*/
	/*--------------------------------------------------------------------------*/
	Mcp2515WriteCanId(MCP_RXM0SIDH,filterCfg->ext,filterCfg->mask0);
	Mcp2515WriteCanId(MCP_RXM1SIDH,filterCfg->ext,filterCfg->mask1);
	Mcp2515WriteCanId(MCP_RXF0SIDH,filterCfg->ext,filterCfg->filter0);
	Mcp2515WriteCanId(MCP_RXF1SIDH,filterCfg->ext,filterCfg->filter1);
	Mcp2515WriteCanId(MCP_RXF2SIDH,filterCfg->ext,filterCfg->filter2);
	Mcp2515WriteCanId(MCP_RXF3SIDH,filterCfg->ext,filterCfg->filter3);
	Mcp2515WriteCanId(MCP_RXF4SIDH,filterCfg->ext,filterCfg->filter4);
	Mcp2515WriteCanId(MCP_RXF5SIDH,filterCfg->ext,filterCfg->filter5);
	/*--------------------------------------------------------------------------*/
	/* CAN TX buffers clear														*/
	/*--------------------------------------------------------------------------*/
	cmdWrite3[2] = 0;								// data to write is 0
    for (i = 0; i < 14; i++) 						// full TX reg
	{
		cmdWrite3[1] = MCP_TXB0CTRL + i;			// for each TX reg
		Spi_Write(cmdWrite3,sizeof(cmdWrite3));		// clear it
		cmdWrite3[1] = MCP_TXB1CTRL + i;			// for each TX reg
		Spi_Write(cmdWrite3,sizeof(cmdWrite3));		// clear it
		cmdWrite3[1] = MCP_TXB2CTRL + i;			// for each TX reg
		Spi_Write(cmdWrite3,sizeof(cmdWrite3));		// clear it
    }
	/*--------------------------------------------------------------------------*/
	/* CAN RX buffers clear	& deactivate										*/
	/*--------------------------------------------------------------------------*/
	cmdWrite3[2] = 0;								// data to write is 0
	cmdWrite3[1] = MCP_RXB0CTRL;					// for RXB0 buffer
	Spi_Write(cmdWrite3,sizeof(cmdWrite3));			// clear it	
	cmdWrite3[1] = MCP_RXB1CTRL;					// for RXB1 buffer
	Spi_Write(cmdWrite3,sizeof(cmdWrite3));			// clear it	
	/*--------------------------------------------------------------------------*/
	/* RXB0 buffers enabled for STD identifier with rollover on buffer 1		*/
	/*--------------------------------------------------------------------------*/
	cmdMask4[1] = MCP_RXB0CTRL;							// RXB0CTRL
	cmdMask4[2] = MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK;	// MASK ONLY USED BITS
	if(filterCfg->ext == 0)								// std identifiers
	{
		cmdMask4[3] = MCP_RXB_RX_STD | MCP_RXB_BUKT_MASK;	// STD and ROLLOVER
	}
	else												// ext. identifiers
	{
		cmdMask4[3] = MCP_RXB_RX_EXT | MCP_RXB_BUKT_MASK;	// EXT and ROLLOVER
	}
	Spi_Write(cmdMask4,sizeof(cmdMask4));
	/*--------------------------------------------------------------------------*/
	/* RXB1 buffers enabled for STD identifier 									*/
	/*--------------------------------------------------------------------------*/
	cmdMask4[1] = MCP_RXB1CTRL;							// RXB0CTRL
	cmdMask4[2] = MCP_RXB_RX_MASK;						// MASK ONLY USED BITS
	if(filterCfg->ext == 0)								// std identifiers
	{
		cmdMask4[3] = MCP_RXB_RX_STD;						// STD identifiers
	}
	else												// ext. identifiers
	{
		cmdMask4[3] = MCP_RXB_RX_EXT;						// EXT identifiers
	}
	Spi_Write(cmdMask4,sizeof(cmdMask4));	
	/*--------------------------------------------------------------------------*/
	/* Enabled interrupts on receive		 									*/
	/*--------------------------------------------------------------------------*/
	cmdMask4[0] = MCP_BITMOD;								// BIT MODIFY
	cmdMask4[1] = MCP_CANINTE;							// INTE register
	cmdMask4[2] = 0x1F;											// MASK RX & TX INTERRUPTS
	cmdMask4[3] = 0x1F;											// ENABLE RX & TX INTERRUPTS
	Spi_Write(cmdMask4,sizeof(cmdMask4));	
	/*--------------------------------------------------------------------------*/
	/* Enabled normal mode 					 									*/
	/*--------------------------------------------------------------------------*/
	cmdMask4[0] = MCP_BITMOD;							// BIT MODIFY
	cmdMask4[1] = MCP_CANCTRL;							// CTRLREG
	cmdMask4[2] = MODE_MASK;							// MASK ONLY USED BITS
	cmdMask4[3] = MODE_NORMAL;							// MODE IS NORMAL
	Spi_Write(cmdMask4,sizeof(cmdMask4));	

}

void Mcp2515Sleep(void)
{
    	uint8_t cmdMask4[]={0,0,0};

	cmdMask4[0] = MCP_WRITE;							// BIT MODIFY
	cmdMask4[1] = MCP_CANCTRL;							// CTRLREG
	cmdMask4[2] = MODE_SLEEP;							// MODE IS NORMAL
	Spi_Write(cmdMask4,sizeof(cmdMask4));

}

/******************************************************************************/
/* FUNCTION		: Mcp2515ReadCanId                                              */
/*----------------------------------------------------------------------------*/
/* INPUTS		: adress_of_RX_buffer, *extended, *can_id                         */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Read the can_id of a received message                           */
/******************************************************************************/
void Mcp2515ReadCanId( const uint8_t mcp_addr,
	uint8_t* ext, uint32_t* can_id )
{
	uint8_t msg[]={MCP_READ,0,0,0,0,0};			// a read command of 4 bytes
	
	msg[1] = mcp_addr;							// get RX address
    *ext = 0;									// default no ext
    *can_id = 0;								// default ID is 0
    Spi_ReadWrite(msg,sizeof(msg));				// read RX buffer
	
	*can_id = (msg[MCP_SIDH+2]<<3) + (msg[MCP_SIDL+2]>>5);	// gets CAN id
	
    if ( (msg[MCP_SIDL+2] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M ) // is extended
	{
		// extended id
        *can_id = (*can_id<<2) + (msg[MCP_SIDL+2] & 0x03);	// compute it
        *can_id <<= 16;
        *can_id = *can_id +(msg[MCP_EID8+2]<<8) + msg[MCP_EID0+2];
        *ext = 1;
    }
}

/******************************************************************************/
/* FUNCTION		: Mcp2515ReadCanMsg                                             */
/*----------------------------------------------------------------------------*/
/* INPUTS		: adress_of_RX_buffer_SIDH, *can_message                          */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Read the full CAN message received                              */
/******************************************************************************/
void Mcp2515ReadCanMsg( const uint8_t mcp_addr,
	struct CANMESSAGE* msg)
{
	uint8_t cmd[10];
	
	/*--------------------------------------------------------------------------*/
	/* read CAN identifier														*/
	/*--------------------------------------------------------------------------*/
    Mcp2515ReadCanId( mcp_addr, &(msg->extended_identifier), 
		&(msg->identifier) );
	/*--------------------------------------------------------------------------*/    
	cmd[0] = MCP_READ;					// read command
	cmd[1] = mcp_addr-1;				// adress of control register
    Spi_ReadWrite(cmd,3);				// read control register
    msg->rtr = 0;						// default RTR is 0
	if (cmd[2] & 0x08) 					// is it a RTR message
	{
        msg->rtr = 1;					// set RTR bit
    } 
	/*--------------------------------------------------------------------------*/    
	cmd[0] = MCP_READ;					// read command
	cmd[1] = mcp_addr+4;				// address of DLC
    Spi_ReadWrite(cmd,3);				// read DLC
    msg->dlc = cmd[2] & MCP_DLC_MASK;	// copy it in msg
	/*--------------------------------------------------------------------------*/        
	cmd[0] = MCP_READ;					// read command
	cmd[1] = mcp_addr+5;				// adress of data
    Spi_ReadWrite(cmd,msg->dlc + 2);	// read length of data
	memcpy((void *)msg->dta,(const void *)&cmd[2],msg->dlc);	// copy them in msg
}

/******************************************************************************/
/* FUNCTION		: Mcp2515WriteCanId                                             */
/*----------------------------------------------------------------------------*/
/* INPUTS		: adress_of_TX_buffer, extended, can_id                           */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Write the can_id for a message to send                          */
/******************************************************************************/
void Mcp2515WriteCanId(uint8_t mcp_addr, uint8_t ext,uint32_t can_id )
{
    uint16_t canid;
	uint8_t buf[6];
	
    canid = (uint16_t)(can_id & 0x0FFFF);		// mask 16 LSB
    
	buf[0] = MCP_WRITE;						// write operation
	buf[1] = mcp_addr;						// at TX_BUFFER address
	/*--------------------------------------------------------------------------*/        
	if ( ext == 1) 							// identifier is extended (29 bits)
	{
        buf[MCP_EID0+2] = (uint8_t) (canid & 0xFF);	// 8 LSB of identifier
        buf[MCP_EID8+2] = (uint8_t) (canid >> 8);		// 8 MSB of identifier
        canid = (uint16_t)( can_id / 0x10000L );		// gets high 13 bits
        buf[MCP_SIDL+2] = (uint8_t) (canid & 0x03);		// copy at right place
        buf[MCP_SIDL+2] += (uint8_t) ((canid & 0x1C )*8);	// copy at right place
        buf[MCP_SIDL+2] |= MCP_TXB_EXIDE_M;				// copy at right place
        buf[MCP_SIDH+2] = (uint8_t) (canid / 32 );		// copy at right place
    }
	/*--------------------------------------------------------------------------*/        
    else 									// identifier is standard (11 bits)
	{
        buf[MCP_SIDH+2] = (uint8_t) (canid >> 3);			// copy STD_ID at place
        buf[MCP_SIDL+2] = (uint8_t) ((canid & 0x07 )*32);	// copy STD_ID at place
        buf[MCP_EID0+2] = 0;							// no extended
        buf[MCP_EID8+2] = 0;							// no extended
    }
    Spi_Write(buf,6);						// write can_id
}

/******************************************************************************/
/* FUNCTION		: Mcp2515ReadCanMsg                                             */
/*----------------------------------------------------------------------------*/
/* INPUTS		: adress_of_TX_buffer_SIDH, *can_message                          */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Read the full CAN message received                              */
/******************************************************************************/
void Mcp2515WriteCanMsg( const uint8_t buffer_sidh_addr,
	const struct CANMESSAGE* msg)
{
    uint8_t mcp_addr, i, dlc;
	uint8_t	buf[10];
	mcp_addr = buffer_sidh_addr;

	buf[0] = MCP_WRITE;					// write command
	buf[1] = mcp_addr+5;				// data pointer
	/*--------------------------------------------------------------------------*/        
	for(i=0;i<msg->dlc;i++)				// copy data to TX buffer
	{
		buf[i+2] = msg->dta[i];
	}
    Spi_Write(buf,msg->dlc + 2);
	/*--------------------------------------------------------------------------*/        	
    Mcp2515WriteCanId( mcp_addr, 		// write identifier
		msg->extended_identifier,
		msg->identifier ); 
	dlc = msg->dlc;						// get DLC
	/*--------------------------------------------------------------------------*/        
	if ( msg->rtr == 1)					// if RTR mode
	{
		dlc = msg->dlc | MCP_RTR_MASK;  // set bit in byte
	}
	buf[0] = MCP_WRITE;					// write command
	buf[1] = mcp_addr+4;				// DLC pointer
	buf[2] = dlc;						// DLC data
    Spi_Write(buf,3);					// write the RTR and DLC
}

/******************************************************************************/
/* FUNCTION		: Mcp2515GetNextFreeTXBuf                                       */
/*----------------------------------------------------------------------------*/
/* INPUTS		: * free_tx_buffer                                                */
/* OUTPUTS		: -                                                             */
/*----------------------------------------------------------------------------*/
/* GOAL			: Gets the next free TX buffer                                    */
/******************************************************************************/
uint8_t Mcp2515GetNextFreeTXBuf(uint8_t *txbuf_n)
{
	uint8_t i;
	uint8_t ctrlregs[MCP_N_TXBUFFERS] = {MCP_TXB0CTRL,MCP_TXB1CTRL,MCP_TXB2CTRL};
	uint8_t buf[3];
	*txbuf_n = 0x00;
	
	/*--------------------------------------------------------------------------*/        
	for (i=0; i<MCP_N_TXBUFFERS; i++) 		// check all three TX buffers
	{
		buf[0] = MCP_READ;					// read command
		buf[1] = ctrlregs[i];				// Ctrl TX register address
		Spi_ReadWrite(buf,3);				// read the TX status
		
		if ( (buf[2] & MCP_TXB_TXREQ_M) == 0 ) // TX buffer is empty
		{
			*txbuf_n = ctrlregs[i]+1; 		// return SIDH-address of Buffer
			return MCP2515_OK;				// return OK
		}
	}
	return MCP_ALLTXBUSY;					// all TX buffers are busy
}


