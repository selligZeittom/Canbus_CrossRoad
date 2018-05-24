#ifndef MCP2515_H_
#define MCP2515_H_

#define MCPDEBUG (1)
#define MCPDEBUG_TXBUF (0)

#include "mcp2515_defs.h"
//#include "can.h"

#define MCP_CS_PORT PORTB
#define MCP_CS_DDR  DDRB
#define MCP_CS_BIT  PB0

#define MCP_N_TXBUFFERS (3)

#define MCP_RXBUF_0 (MCP_RXB0SIDH)
#define MCP_RXBUF_1 (MCP_RXB1SIDH)

#define MCP_TXBUF_0 (MCP_TXB0SIDH)
#define MCP_TXBUF_1 (MCP_TXB1SIDH)
#define MCP_TXBUF_2 (MCP_TXB2SIDH)

// #define MCP2515_SELECT() (SPI_SS_LOW())
// #define MCP2515_UNSELECT() (SPI_SS_HIGH())

#define MCP2515_SELECT()   ( MCP_CS_PORT &= ~(1<<MCP_CS_BIT) )
#define MCP2515_UNSELECT() ( MCP_CS_PORT |=  (1<<MCP_CS_BIT) )

#define MCP2515_OK         (0)
#define MCP2515_FAIL       (1)
#define MCP_ALLTXBUSY      (2)


void mcp2515_reset(void);

uint8_t mcp2515_readRegister(const uint8_t address);
void mcp2515_setRegisterS(const uint8_t address,
	const uint8_t values[],
	const uint8_t n);

void mcp2515_setRegister(const uint8_t address,
	const uint8_t value);
void mcp2515_setRegisterS(const uint8_t address,
	const uint8_t values[],
	const uint8_t n);
void mcp2515_modifyRegister(const uint8_t address, const uint8_t mask,
	const uint8_t data);

uint8_t mcp2515_readStatus(void);
uint8_t mcp2515_RXStatus(void);

uint8_t mcp2515_setCANCTRL_Mode(const uint8_t newmode);

void Mcp2515_Init(const struct CANSPEED * speedCfg,struct CANFILTER * filterCfg);
void Mcp2515WriteCanId(uint8_t mcp_addr, uint8_t ext,uint32_t can_id );

//void Mcp2515WriteCanId(UINT8 mcp_addr, UINT8 ext,UINT32 can_id );

//void mcp2515_write_can_id( const UINT8 mcp_addr, 
//	const UINT8 ext, const UINT32 can_id );
void mcp2515_read_can_id( const uint8_t mcp_addr,
	uint8_t* ext, uint32_t* can_id );

void Mcp2515ReadCanMsg( const uint8_t buffer_sidh_addr,
	struct CANMESSAGE* msg);	
//SAP void mcp2515_write_canMsg( const UINT8 buffer_sidh_addr, 
//SAP 	const CanMessage* msg);
//SAP void mcp2515_read_canMsg( const UINT8 buffer_sidh_addr,
//SAP 	CanMessage* msg);
void Mcp2515WriteCanMsg( const uint8_t buffer_sidh_addr,
	const struct CANMESSAGE* msg);

void mcp2515_start_transmit(const uint8_t mcp_addr);

uint8_t Mcp2515GetNextFreeTXBuf(uint8_t *txbuf_n);

void Mcp2515Sleep(void);

#ifdef MCPDEBUG
void mcp2515_dumpExtendedStatus(void);
#endif

#endif
