#pragma once
#include "spi/bus.h"
#include "driver/gpio.h"
#include "mcp23s17.h"

#define MF_PN512_POWERUP_DELAY			100			// Dit is de tijd die de kaart krijgt om op te laden
#define MF_PN512_REG_RFCCFG_VALUE 		0x49
#define MF_PN512_REG_RXTHRESHOLD_VALUE	0x55		// Deze staat bij de KC160 op 0xA5
#define MF_PN512_REG_GSNON_VALUE		0xF4		//	Deze staat bij een KC160 op 0x24	(Let op!! aanpassen van deze waarde heeft gevolgen voor het stroomverbruik!)
#define MF_PN512_REG_CWGSP_VALUE		0x3F		//	Deze staat bij een KC160 op 0x09	(Let op!! aanpassen van deze waarde heeft gevolgen voor het stroomverbruik!)


#define PN512_MODE_LOW_POWER	0x1
#define PN512_MODE_ALL			0x2

#define MF_PN512_COMIRQ_TIMER	0x1
#define MF_PN512_COMIRQ_ERROR	0x2
#define MF_PN512_COMIRQ_LOALERT	0x4
#define MF_PN512_COMIRQ_HIALERT	0x8
#define MF_PN512_COMIRQ_IDLE	0x10
#define MF_PN512_COMIRQ_RXDONE	0x20
#define MF_PN512_COMIRQ_TXDONE	0x40
#define MF_PN512_COMIRQ_SET		0x80

/***************************************************
****************************************************
			MF_PN512 commands
****************************************************
***************************************************/
#define MF_PN512_CMD_IDLE				0x0
#define MF_PN512_CMD_CONFIG				0x1
#define MF_PN512_CMD_GENERATERANDOMID	0x2
#define MF_PN512_CMD_CALCCRC			0x3
#define MF_PN512_CMD_TRANSMIT			0x4
#define MF_PN512_CMD_NOCMDCHANGE		0x7
#define MF_PN512_CMD_RECEIVE			0x8
#define MF_PN512_CMD_TRANCEIVE			0xC
#define MF_PN512_CMD_AUTOCOLL			0xD
#define MF_PN512_CMD_MFAUTHENT			0xE
#define MF_PN512_CMD_SOFTRESET			0xF

/***************************************************
****************************************************
			MF_PN512 REGISTERS
****************************************************
***************************************************/
//page 0
#define	MF_PN512_REG_PAGE0			0x00
#define	MF_PN512_REG_COMMAND		0x01
#define	MF_PN512_REG_COMLEN			0x02
#define	MF_PN512_REG_DIVLEN			0x03
#define	MF_PN512_REG_COMIRQ			0x04
#define	MF_PN512_REG_DIVIRQ			0x05
#define	MF_PN512_REG_ERROR			0x06
#define	MF_PN512_REG_STATUS1		0x07
#define	MF_PN512_REG_STATUS2		0x08
#define	MF_PN512_REG_FIFODATA		0x09
#define	MF_PN512_REG_FIFOLEVEL		0x0A
#define	MF_PN512_REG_WATERLEVEL		0x0B
#define	MF_PN512_REG_CONTROL		0x0C
#define	MF_PN512_REG_BITFRAMING		0x0D
#define	MF_PN512_REG_COLL			0x0E
#define	MF_PN512_REG_RFU0			0x0F
//page 1
#define	MF_PN512_REG_PAGE1			0x10
#define	MF_PN512_REG_MODE			0x11
#define	MF_PN512_REG_TXMODE			0x12
#define	MF_PN512_REG_RXMODE			0x13
#define	MF_PN512_REG_TXCONTROL		0x14
#define	MF_PN512_REG_TXAUTO			0x15
#define	MF_PN512_REG_TXSEL			0x16
#define	MF_PN512_REG_RXSEL			0x17
#define	MF_PN512_REG_RXTHRESHOLD	0x18
#define	MF_PN512_REG_DEMOD			0x19
#define	MF_PN512_REG_FELNFC1		0x1A
#define	MF_PN512_REG_FELNFC2		0x1B
#define	MF_PN512_REG_MIFNFC			0x1C
#define	MF_PN512_REG_MANUALRCV		0x1D
#define	MF_PN512_REG_TYPEB			0x1E
#define	MF_PN512_REG_SERIALSPEED	0x1F
//page 2
#define	MF_PN512_REG_PAGE2			0x20
#define	MF_PN512_REG_CRCRESULT0		0x21
#define	MF_PN512_REG_CRCRESULT1		0x22
#define	MF_PN512_REG_GSNOOFF		0x23
#define	MF_PN512_REG_MODWIDTH		0x24
#define	MF_PN512_REG_TXBITPHASE		0x25
#define	MF_PN512_REG_RFCCFG			0x26
#define	MF_PN512_REG_GSNON			0x27
#define	MF_PN512_REG_CWGSP			0x28
#define	MF_PN512_REG_MODGSP			0x29
#define	MF_PN512_REG_TMODE			0x2A
#define	MF_PN512_REG_TPRESCALER		0x2B
#define	MF_PN512_REG_TRELOAD0		0x2C
#define	MF_PN512_REG_TRELOAD1		0x2D
#define	MF_PN512_REG_TCOUNTERVAL0	0x2E
#define	MF_PN512_REG_TCOUNTERVAL1	0x2F
//page 3
#define	MF_PN512_REG_PAGE3			0x30
#define	MF_PN512_REG_TESTSEL1		0x31
#define	MF_PN512_REG_TESTSEL2		0x32
#define	MF_PN512_REG_TESTPINEN		0x33
#define	MF_PN512_REG_TESTPINVALUE	0x34
#define	MF_PN512_REG_TESTBUS		0x35
#define	MF_PN512_REG_AUTOTEST		0x36
#define	MF_PN512_REG_VERSION		0x37
#define	MF_PN512_REG_ANALOGTEST		0x38
#define	MF_PN512_REG_TESTDAC1		0x39
#define	MF_PN512_REG_TESTDAC2		0x3A
#define	MF_PN512_REG_TESTADC		0x3B
#define	MF_PN512_REG_RFT0			0x3C
#define	MF_PN512_REG_RFT1			0x3D
#define	MF_PN512_REG_RFT2			0x3E
#define	MF_PN512_REG_RFT3			0x3F

class PN512
{
	SPI::Device spidev;
	uint8_t devAddr = 0;
//	Task irqTask;
	gpio_num_t irqPin = GPIO_NUM_NC;
	
private:
	MCP23S17* expander;
	uint16_t AntiColl(uint8_t antiCollCode, uint8_t * snr);
	uint16_t Select(uint8_t antiCollCode, uint8_t * snr, uint8_t * sak);
	uint16_t AuthKey(uint8_t auth_mode, uint8_t *snr, uint8_t *keys, uint8_t block);
	
	void InitTimer(uint16_t m_sec_X100);
	void RESET_MIFARE(void);
	void START_MIFARE(void);
	void FLUSH(void);
	void ConfigAsInitiator(void);
	void Init2(void);
	uint16_t WaitForTranceive(uint16_t escape);
	void PrepareTranceive(void);
	
	void WriteRC(uint8_t address, uint8_t data);
	uint8_t ReadRC(uint8_t address);
	
	esp_err_t Transmit(uint8_t * txData, uint8_t * rxData, uint8_t count);
	
public:
	bool Init(SPI::Bus* spiBus, gpio_num_t cs, gpio_num_t irq, transaction_cb_t pre_cb, transaction_cb_t post_cb, MCP23S17* expander);
	uint8_t GetVersion(void);
	//
	void RFon(void);
	void RFoff(void);
	void SOFTPOWERDOWN(void);
	void WAKEUP(void);

	uint16_t Request(uint8_t request, uint8_t * atq);
	uint16_t Read(uint8_t cmd, uint8_t addr, uint8_t datalen, uint8_t *data);
	uint16_t Write(uint8_t cmd, uint8_t addr, uint8_t datalen, uint8_t *data);
};