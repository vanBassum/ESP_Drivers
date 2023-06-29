#include "pn512.h"
#include "task.h"

static const char *TAG = "PN512";

/*****************************************************************************
** Function name:		void MF_PN512_Init()
**
** Descriptions:		Initialiseer de PN512 chip
**
** parameters:			none
** Returned value:		None
** 
*****************************************************************************/

bool PN512::Init(SPI::Bus* spiBus, gpio_num_t cs, gpio_num_t irq, transaction_cb_t pre_cb, transaction_cb_t post_cb, MCP23S17* expander)
{
	this->expander = expander;
	irqPin = irq;
	
	bool result;
	spi_device_interface_config_t spi_devcfg;
	memset(&spi_devcfg, 0, sizeof(spi_devcfg));
	spi_devcfg.address_bits = 0;
	spi_devcfg.command_bits = 0;
	spi_devcfg.dummy_bits = 0;
	spi_devcfg.mode = 0;
	spi_devcfg.duty_cycle_pos = 0;
	spi_devcfg.cs_ena_posttrans = 0;
	spi_devcfg.cs_ena_pretrans = 0;
	spi_devcfg.clock_speed_hz = 2 * 1000 * 1000; 
	spi_devcfg.flags = 0;
	spi_devcfg.queue_size = 7;
	spi_devcfg.spics_io_num = cs;
	spi_devcfg.pre_cb = pre_cb;
	spi_devcfg.post_cb = post_cb;
	result = spidev.Init(spiBus, &spi_devcfg);	
	
	//init reset pin

	// TODO
	// Reset pin is connected to MCP23S17 highest bit
	expander->SetPinsMode(MCP23S17_PIN_B5, MCP23S17_PINMODE_OUTPUT);
	expander->SetPinsMode(MCP23S17_PIN_B6, MCP23S17_PINMODE_OUTPUT);
	expander->SetPinsMode(MCP23S17_PIN_B7, MCP23S17_PINMODE_OUTPUT);
	expander->SetPins(MCP23S17_PIN_B5, MCP23S17_PIN_B5); //	RESET_MIFARE();
	expander->SetPins(MCP23S17_PIN_B6, MCP23S17_PIN_B6); //	RESET_MIFARE();
	expander->SetPins(MCP23S17_PIN_B7, MCP23S17_PIN_B7); //	RESET_MIFARE();
	vTaskDelay(pdMS_TO_TICKS(10)); //SleepMs(10); // wacht even
	expander->SetPins(MCP23S17_PIN_B5, MCP23S17_PIN_NONE); //START_MIFARE();
	expander->SetPins(MCP23S17_PIN_B6, MCP23S17_PIN_NONE);
	expander->SetPins(MCP23S17_PIN_B7, MCP23S17_PIN_NONE);
	vTaskDelay(pdMS_TO_TICKS(1));  //SleepUs(100); // Wacht tot het IC is opgestart
	return result;
}

esp_err_t PN512::Transmit(uint8_t * txData, uint8_t * rxData, uint8_t count)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       			
	t.length = (count * 8);              		
	t.tx_buffer = txData;               		
	t.rx_buffer = rxData;
	return spidev.PollingTransmit(&t);  		
}


/*****************************************************************************
** Function name:		uint8_t PN512::ReadRC(uint8_t Address)
**
** Descriptions:		Lees data van de RC512 via page selecting
**
** parameters:			uint8_t adress	: adres waar de data in moet komen
**						
** Returned value:		uint8_t : de te lezen byte
** 
*****************************************************************************/
uint8_t PN512::ReadRC(uint8_t address)
{
//	spidev.AcquireBus();
	uint8_t txData[2];
	uint8_t rxData[2];	
	txData[0] = (((address << 1) | 0x80) & 0xFE);
	txData[1] = 0x00;
	Transmit(txData, rxData, 2);
	return rxData[1];
//	spidev.ReleaseBus();
}  

/*****************************************************************************
** Function name:		void PN512::WriteRC(uint8_t Address, uint8_t value)
**
** Descriptions:		Schrijf data naar RC512 via pageselecting
**
** parameters:			uint8_t adress	: adres waar de data in moet komen
**						uint8_t value		: data die daar moet komen
** Returned value:		None
** 
*****************************************************************************/
void PN512::WriteRC(uint8_t address, uint8_t data)
{
	spidev.AcquireBus();
	uint8_t txData[2];
	txData[0] = ((address << 1) & 0x7E);
	txData[1] = data;
	Transmit(txData, NULL, 2);
	spidev.ReleaseBus();
}

/*****************************************************************************
** Function name:		void PN512::RFon()
**
** Descriptions:		Zet het veld aan
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void PN512::RFon(void)
{
	WriteRC(MF_PN512_REG_TXCONTROL, 0x83); // Enable field
}

/*****************************************************************************
** Function name:		void PN512::RFoff( )
**
** Descriptions:		Zet het veld uit
**
** parameters:			none
** Returned value:		None
** 
*****************************************************************************/
void PN512::RFoff(void)
{
	//PN512::WriteRC(SPIBus,DeviceNmr,PN512::REG_TXCONTROL,0x03);	// dISable field
	WriteRC(MF_PN512_REG_TXCONTROL, 0x00); // dISable field
}

/*****************************************************************************
** Function name:		void InitTimer(unsigned int m_sec_X100 )
**
** Descriptions:		Initialiseer de timer
**
** parameters:			unsigned int m_sec_X100 = Het aantal msecs..
** Returned value:		None
** 
*****************************************************************************/
void PN512::InitTimer(uint16_t m_sec_X100)
{
	WriteRC(MF_PN512_REG_TMODE, 0x82);
	WriteRC(MF_PN512_REG_TPRESCALER, 0xA5);	
	WriteRC(MF_PN512_REG_TRELOAD0, ((m_sec_X100 >> 8) & 0xFF));
	WriteRC(MF_PN512_REG_TRELOAD1, (m_sec_X100 & 0xFF));
}

void PN512::SOFTPOWERDOWN(void)
{
	RFoff(); // Zet het veld weer uit (als er een kaart in het veld zit blijft het veld aan staan)		
	WriteRC(MF_PN512_REG_COMMAND, 0x10);
}

void PN512::WAKEUP(void)
{
	uint16_t escape = 500;
	WriteRC(MF_PN512_REG_COMMAND, 0x00);
	//wacht tot aan
	while (escape > 0) 	//i = max tijd wachten 500*10 = 5 ms
	{	
		if (((ReadRC(MF_PN512_REG_COMMAND) & 0x10) == 0x00)) 
			break;
		vTaskDelay(pdMS_TO_TICKS(1)); //SleepUs(10);
		escape--;		
	}
	;   
}

void PN512::RESET_MIFARE(void)
{
	// MF_PN512_SPI_DEVICE0_READER_RESET;
	// nrf_gpio_pin_clear(MF_PN512_SPI0_DEVICE0_READER_RESETPIN);
//TODO
}

void PN512::START_MIFARE(void)
{
	//nrf_gpio_pin_set(MF_PN512_SPI0_DEVICE0_READER_RESETPIN); 
	//todo
}

/*****************************************************************************
** Function name:		void PN512_FLUSH()
**
** Descriptions:		flush de PN512
**
** parameters:			none
** Returned value:		None
** 
*****************************************************************************/
void PN512::FLUSH(void)
{
	//flushing
	WriteRC(MF_PN512_REG_COMIRQ, 0x7F); //reset all IRQ	 
	WriteRC(MF_PN512_REG_DIVIRQ, 0x00); //reset all IRQ	 					
	WriteRC(MF_PN512_REG_FIFOLEVEL, 0x80); //flush
	WriteRC(MF_PN512_REG_COMIRQ, 0x01); //reset all IRQ	 
	WriteRC(MF_PN512_REG_BITFRAMING, 0x00);	
}

/*****************************************************************************
** Function name:		uint8_t MF_PN512_GetVersion()
**
** Descriptions:		Geeft versie nummer terug van de aangesloten mifare chip
**
** parameters:			none
** Returned value:		None
** 
*****************************************************************************/
uint8_t PN512::GetVersion(void)
{
	return ReadRC(MF_PN512_REG_VERSION);	
}

/*****************************************************************************
** Function name:		void MF_PN512_ConfigAsInitiator()
**
** Descriptions:		Initialiseer de PN512 chip als initiator
**									Dit is volledig nagebouwd van de philips applicatie NFC-Tools-Setup-V4_1.exe
**
** parameters:			none
** Returned value:		None
** 
*****************************************************************************/
void PN512::ConfigAsInitiator(void)
{	
	WriteRC(MF_PN512_REG_COMMAND, MF_PN512_CMD_SOFTRESET); // Soft reset
	vTaskDelay(pdMS_TO_TICKS(1)); //SleepUs(50); // Wacht tot het IC is opgestart
#ifdef PN512_FORCE_DEFAULT_VALUES	
	WriteRC MF_PN512_REG_TXCONTROL, 0x80); // default value
	WriteRC( MF_PN512_REG_STATUS2, 0x00); // default value
#endif	
	WriteRC(MF_PN512_REG_TXMODE, 0x80); // Enable CRC
	WriteRC(MF_PN512_REG_RXMODE, 0x80); // Enable CRC
#ifdef PN512_FORCE_DEFAULT_VALUES	
	WriteRC(MF_PN512_REG_TXAUTO, 0x00); // default value
	WriteRC(MF_PN512_REG_MODWIDTH, 0x26); // default value
#endif	
	WriteRC(MF_PN512_REG_DEMOD, 0x4D); // Sets PLL and IQ
	WriteRC(MF_PN512_REG_RFCCFG, MF_PN512_REG_RFCCFG_VALUE); // Sets RX GAIN and level detection
	WriteRC(MF_PN512_REG_TXAUTO, 0x40); // forces a 100% ASK modulation
	WriteRC(MF_PN512_REG_RXTHRESHOLD, MF_PN512_REG_RXTHRESHOLD_VALUE); // Defines the minimum signal strengths
#ifdef PN512_FORCE_DEFAULT_VALUES	
	WriteRC MF_PN512_REG_MODWIDTH, 0x26); // default value
#endif	
	WriteRC(MF_PN512_REG_TXBITPHASE, 0x8F); // 
	
	WriteRC(MF_PN512_REG_GSNON, MF_PN512_REG_GSNON_VALUE); // conductance while sending	 N-driver	nieuwe antenne  (vroeger had deze waarde 0xF4)
	WriteRC(MF_PN512_REG_CWGSP, MF_PN512_REG_CWGSP_VALUE); // conductance P-driver 0x0 tot 0x3F  nieuwe antenne		
	
	WriteRC(MF_PN512_REG_MODGSP, 0x11); // heeft invloed op het verbruik
	WriteRC(MF_PN512_REG_CONTROL, 0x10); // the PN512 acts as initiator
	WriteRC(MF_PN512_REG_TXAUTO, 0x47); // default value
	WriteRC(MF_PN512_REG_DIVLEN, 0x80); // IRQPushPull. bit 7: Set to logic 1, the pin IRQ works as standard CMOS output pad
	WriteRC(MF_PN512_REG_COMMAND, MF_PN512_CMD_IDLE); // IDLE
}


void PN512::Init2(void)
{	
	RESET_MIFARE();
	vTaskDelay(pdMS_TO_TICKS(10)); //SleepMs(10); // wacht even
	START_MIFARE();
	vTaskDelay(pdMS_TO_TICKS(1)); //SleepUs(100); // Wacht tot het IC is opgestart	
	ConfigAsInitiator();
}

uint16_t PN512::WaitForTranceive(uint16_t escape)
{
	uint8_t intr=0;
	while (escape > 0) 
	{
		intr = ReadRC(MF_PN512_REG_COMIRQ);		
		if (((intr & (MF_PN512_COMIRQ_RXDONE | MF_PN512_COMIRQ_TXDONE)) == (MF_PN512_COMIRQ_RXDONE | MF_PN512_COMIRQ_TXDONE)) 
				|| 	((intr & MF_PN512_COMIRQ_TIMER) == MF_PN512_COMIRQ_TIMER) 
				|| 	((intr & MF_PN512_COMIRQ_ERROR) == MF_PN512_COMIRQ_ERROR))
			break;		
		vTaskDelay(pdMS_TO_TICKS(1)); //SleepUs(10);		
		escape--;
	}
	;
	WriteRC(MF_PN512_REG_COMLEN, 0x80);
	WriteRC(MF_PN512_REG_DIVLEN, 0x80);		
	return (((intr & 0x01) != 0x01) && (escape > 0));
}

void PN512::PrepareTranceive(void)
{
	WriteRC(MF_PN512_REG_COMMAND, MF_PN512_CMD_TRANCEIVE); //Start Tranceiver mode
	WriteRC(MF_PN512_REG_COMIRQ, 0x7F); //reset all IRQ
	WriteRC(MF_PN512_REG_DIVIRQ, 0x00); //reset all IRQ	 					
	WriteRC(MF_PN512_REG_FIFOLEVEL, 0x80); //clear fifo
	WriteRC(MF_PN512_REG_COMLEN, 0xA3); //zet de gewenste interupts
	WriteRC(MF_PN512_REG_DIVLEN, 0x80); //zet de gewenste interupts	
}

/*****************************************************************************
** Function name:		int MF_PN512_Request(uint8_t request,uint8_t * atq)
**
** Descriptions:		Mifaire request 
**
** parameters:			uint8_t *atq = antwoord 
** Returned value:		error code
** 
*****************************************************************************/
uint16_t PN512::Request(uint8_t request, uint8_t * atq)
{	
	uint16_t count, errors = 0x100;
	ConfigAsInitiator();
	atq[0] = 0x00;
	atq[1] = 0x00;
	WriteRC(MF_PN512_REG_STATUS2, 0x00);
	WriteRC(MF_PN512_REG_COLL, 0x80);
	WriteRC(MF_PN512_REG_TXMODE, 0x00);
	WriteRC(MF_PN512_REG_RXMODE, 0x00);
	WriteRC(MF_PN512_REG_BITFRAMING, 0x07);
	PrepareTranceive();
	RFon(); // Zet het veld aan
#ifndef MF_POWERUP_DELAY
#define MF_POWERUP_DELAY 50
#endif
	vTaskDelay(pdMS_TO_TICKS(1));  //	SleepUs(MF_PN512_POWERUP_DELAY); // gun de kaart wat meer tijd om energie op te nemen
	InitTimer(3); //0,3 ms
	WriteRC(MF_PN512_REG_FIFODATA, request); // FIFO request code
	WriteRC(MF_PN512_REG_BITFRAMING, 0x87); // Sends the data
	if (WaitForTranceive(200))
	{	
		count = ReadRC(MF_PN512_REG_FIFOLEVEL); // readFIFO Level
		errors = ReadRC(MF_PN512_REG_ERROR);
		if (errors == 0)
		{	
			if (count >= 2)
			{
				atq[0] = ReadRC(MF_PN512_REG_FIFODATA); // ATQ LSB
				atq[1] = ReadRC( MF_PN512_REG_FIFODATA); // ATQ MSB					
			}
			else
			{
				errors = 0x200;
			}
		}
	}
	else
	{
		//timeout betekend geen kaarten in het veld			
		errors = 0xFF;
		atq[0] = 0;
		atq[1] = 0;
	}	
	FLUSH();
	return errors;
}

/*****************************************************************************
** Function name:		int MF_PN512_AntiColl(uint8_t * snr )
**
** Descriptions:		Mifare select 
**
** parameters:			uint8_t antiCollCode = code 0x91 | (atq waarden)
**						uint8_t * snr = array van 5 bytes waarin de UID komt te staan
**						
** Returned value:		error code
** 
*****************************************************************************/
uint16_t PN512::AntiColl(uint8_t antiCollCode, uint8_t * snr)
{
	uint16_t count, i, errors = 0x100;
	//> Mifare Anticoll:
	//> ----------------
	PrepareTranceive();
	WriteRC(MF_PN512_REG_FIFODATA, antiCollCode); // anticollision	
	WriteRC(MF_PN512_REG_FIFODATA, 0x20); // aantal te versturen bytes = 0 --> (aantal << 4) + 0x20
	WriteRC(MF_PN512_REG_TXMODE, 0x00);
	WriteRC(MF_PN512_REG_RXMODE, 0x00);
	WriteRC(MF_PN512_REG_BITFRAMING, 0x80); // BitframingReg - StartSend, TxLastBits
	InitTimer(0x0A); //timer instellen op 1000us time-out
	if (WaitForTranceive(200))
	{
		count = ReadRC(MF_PN512_REG_FIFOLEVEL); // readFIFO Level
		errors = ReadRC(MF_PN512_REG_ERROR);
		if (errors == 0)
		{
			if (count >= 5)								// 5 karakters ontvangen?
			{
				for (i = 0; i < 5; i++)								// Haal deze 5 bytes binnen
				{
					snr[i] = ReadRC(MF_PN512_REG_FIFODATA);
				} 
				if (snr[4] == (snr[0] ^ snr[1] ^ snr[2] ^ snr[3]))	 // is het snr ok?
				errors = 0;
			} else
			{ 	
				errors = 0x200;
			}
			;
		}
	}
	return errors;
}

/*****************************************************************************
** Function name:		int PN512::Select(uint8_t * snr, uint8_t * sak)
**
** Descriptions:		Mifare select 
**
** parameters:			uint8_t antiCollCode = code 0x91 | (atq waarden)
**						uint8_t *snr = UID van de kaart
**						uint8_t *sak = hierin komt het kaarttype te staan
** Returned value:		error code
** 
*****************************************************************************/
uint16_t PN512::Select(uint8_t antiCollCode, uint8_t * snr, uint8_t * sak)
{ 	
	uint16_t errors = 0x100;
	uint16_t count, i;
	PrepareTranceive();	
	WriteRC(MF_PN512_REG_FIFODATA, antiCollCode); // aantal te versturen bytes = 0 --> (aantal << 4) + 0x20
	WriteRC(MF_PN512_REG_FIFODATA, 0x70); // aantal te versturen bytes = 0 --> (aantal << 4) + 0x20
	WriteRC(MF_PN512_REG_TXMODE, 0x80);
	WriteRC(MF_PN512_REG_RXMODE, 0x80);
	for (i = 0; i < 5; i++)
	{
		WriteRC(MF_PN512_REG_FIFODATA, snr[i]); // Zet de snr in de fifo
	}
	WriteRC(MF_PN512_REG_BITFRAMING, 0x80); // BitframingReg - StartSend, TxLastBits
	InitTimer(20); //2,0 ms					    //timer instellen op 2000us time-out	
	if (WaitForTranceive(200))
	{		
		count = ReadRC(MF_PN512_REG_FIFOLEVEL); // readFIFO Level
		errors = ReadRC(MF_PN512_REG_ERROR);
		if (errors == 0)
		{
			if (count >= 1)								// 1 karakters ontvangen?
			{
				sak[0] = ReadRC(MF_PN512_REG_FIFODATA);
				errors = 0;
			} else
			{
				errors = 0x200;
			}
		}
	}
	return errors;
}

/*****************************************************************************
** Function name:		int PN512::AuthKey(uint8_t auth_mode, uint8_t *snr, uint8_t *keys, uint8_t block )      
**
** Descriptions:		Mifare AuthKey 
**
** parameters:			uint8_t auth_mode	= toegang selectie via keyA of keyB (0x60 of 0x61)
**						uint8_t *snr = UID van de kaart
**						uint8_t *keys = de sleutel 6 bytes
**						uint8_t block = nummer van het blok wat we willen lezen
** Returned value:		error code
** 
*****************************************************************************/
uint16_t PN512::AuthKey(uint8_t auth_mode, uint8_t *snr, uint8_t *keys, uint8_t block)      
{
	uint16_t i, escape;
	uint16_t errors = 1;
	uint8_t intr, status;
	InitTimer(100); //timer instellen op 10 ms time-out
	WriteRC(MF_PN512_REG_COMMAND, MF_PN512_CMD_IDLE); //Start Tranceive
	WriteRC(MF_PN512_REG_COMIRQ, 0x7F); //reset all IRQ
	WriteRC(MF_PN512_REG_DIVIRQ, 0x00); //reset all IRQ	 					
	WriteRC(MF_PN512_REG_FIFOLEVEL, 0x80); //clear fifo
	WriteRC(MF_PN512_REG_COMLEN, 0x93);
	WriteRC(MF_PN512_REG_DIVLEN, 0x80);
	//Zet de te versturen data in de FIFO
	WriteRC(MF_PN512_REG_FIFODATA, auth_mode);	
	WriteRC(MF_PN512_REG_FIFODATA, block);
	for (i = 0; i < 6; i++)
	{
		WriteRC(MF_PN512_REG_FIFODATA, keys[i]); // Zet de key in de fifo
	}
	for (i = 0; i < 4; i++)
	{
		WriteRC(MF_PN512_REG_FIFODATA, snr[i]); // Zet de snr in de fifo
	}
	WriteRC(MF_PN512_REG_COMMAND, MF_PN512_CMD_MFAUTHENT); //Start Tranceive
		
	  //interupts controleren	
	escape = 40; // 2000; !!!!
	while (escape > 0)
	{	
		intr = ReadRC(MF_PN512_REG_COMIRQ);
		if (((intr & MF_PN512_COMIRQ_IDLE) == MF_PN512_COMIRQ_IDLE) 
				|| ((intr & MF_PN512_COMIRQ_TIMER) == MF_PN512_COMIRQ_TIMER)  
			/*|| ((intr & MF_PN512_COMIRQ_ERROR) != MF_PN512_COMIRQ_ERROR)*/)  
			break;
		vTaskDelay(pdMS_TO_TICKS(1)); //SleepUs(50);		 !!!
		escape--;
	}
	;
	if (escape > 0)
	{	
		WriteRC(MF_PN512_REG_COMLEN, 0x80);
		WriteRC(MF_PN512_REG_DIVLEN, 0x80);
		errors = ReadRC(MF_PN512_REG_ERROR); // Read ErrReg
		FLUSH();	
		if (((intr & MF_PN512_COMIRQ_TIMER) != MF_PN512_COMIRQ_TIMER))				  // geen time-out en geen error
		{	
			if (errors == 0)
			{
				status = ReadRC(MF_PN512_REG_STATUS2);
				if ((status & 0x08) == 0x08)
					errors = 0;
				else
					errors = -4; //MI_AUTHERR todo MfErrNo.h als errorCodes handhaven in heel deze file
			}
		} else
			errors |= 0x300;
	}
	return errors;
}



/*****************************************************************************
** Function name:		int PN512::Read(uint8_t cmd, uint8_t addr, uint8_t datalen, uint8_t *data )
**
** Descriptions:		Lees een block van de kaart
**
** parameters:			uint8_t cmd = Het mifare commando voor het schrijven van data
**						uint8_t addr = Het adres op de kaart waar de data moet worden gelezen
**						uint8_t datalen = De lengte van de meegestuurde array (data)
**						uint8_t *data = De locatie waar de gelezen data ingeschreven gaat worden
** Returned value:		error code
** 
*****************************************************************************/
uint16_t PN512::Read(uint8_t cmd, uint8_t addr, uint8_t datalen, uint8_t *data)
{ 
	uint16_t errors = 0x100, i, count;		
	PrepareTranceive();
	WriteRC(MF_PN512_REG_FIFODATA, cmd);
	WriteRC(MF_PN512_REG_FIFODATA, addr);
	WriteRC(MF_PN512_REG_BITFRAMING, 0x80); // BitframingReg - StartSend, TxLastBits	
	InitTimer(50); //5,0 ms					//timer instellen op 5 ms time-out
	if (WaitForTranceive(200))
	{	
		errors = ReadRC(MF_PN512_REG_ERROR); // Read ErrReg
		if (errors == 0)							//geen errors?
		{
			count  = ReadRC(MF_PN512_REG_FIFOLEVEL); // readFIFO Level
			//staat nu vast eigenlijk is dit afhankelijk van het cmd
			if (count >= datalen)								// 16 karakters ontvangen?
			{
				for (i = 0; i < datalen; i++)								// Haal deze 5 bytes binnen
				{
					data[i] = ReadRC(MF_PN512_REG_FIFODATA);
				} 					
			} else
			{
				errors = 0x200;
			}
			;
		}
		FLUSH();	
	}
	return errors;
}

/*****************************************************************************
** Function name:		int PN512::Write(uint8_t cmd, uint8_t addr, uint8_t datalen, uint8_t *data )
**
** Descriptions:		Schrijf een block op de kaart
**
** parameters:			uint8_t cmd = Het mifare commando voor het schrijven van data
**						uint8_t addr = Het adres op de kaart waar de data moet komen te staan
**						uint8_t datalen = De lengte van de meegestuurde array (data)
**						uint8_t *data = De te schrijven data
** Returned value:		error code
** 
*****************************************************************************/
uint16_t PN512::Write(uint8_t cmd, uint8_t addr, uint8_t datalen, uint8_t *data)
{  	
	uint16_t i;
	uint16_t errors = 0x100, status = 0x0, level;	
	PrepareTranceive();
	WriteRC(MF_PN512_REG_FIFODATA, cmd);
	WriteRC(MF_PN512_REG_FIFODATA, addr);
	WriteRC(MF_PN512_REG_BITFRAMING, 0x80); // BitframingReg - StartSend, TxLastBits
	InitTimer(100); //10,0 ms									//timer instellen op 5 ms time-out
	if (WaitForTranceive(200))
	{
		errors = ReadRC(MF_PN512_REG_ERROR); // Read ErrReg		
		level = ReadRC(MF_PN512_REG_FIFOLEVEL); 
		if (level >= 1)
			status = ReadRC(MF_PN512_REG_FIFODATA); 
		WriteRC(MF_PN512_REG_ERROR, 0x00);
		FLUSH();
		if ((errors & 0xFB) == 0)		//we verwachten een CRC error
		{ 
			if (status == 0x0A)
			{
				PrepareTranceive();
				for (i = 0; i < datalen; i++)
					WriteRC(MF_PN512_REG_FIFODATA, data[i]);
				WriteRC(MF_PN512_REG_BITFRAMING, 0x80); // BitframingReg - StartSend, TxLastBits	
				if (WaitForTranceive(200))
				{
					errors = ReadRC(MF_PN512_REG_ERROR); // Read ErrReg		
					level = ReadRC(MF_PN512_REG_FIFOLEVEL); 
					if (level >= 1)
						status = ReadRC(MF_PN512_REG_FIFODATA); 
					WriteRC(MF_PN512_REG_ERROR, 0x00);
					FLUSH();
					if ((errors & 0xFB) == 0)		//we verwachten een CRC error
					{ 
						if (status == 0x0A)
						{
							errors = 0;
						}
					}
				}
			}
		}
	}
	return errors;
}


/////////////// ----------- hoort eigenlijk niet hier maar voor testen wel leuk ----------------

/*****************************************************************************
** Function name:		 CheckCardLogics( unsigned char SPIBus, unsigned char DeviceNmr )
**
** Descriptions:		Kijkt of er een Mifare 1K kaart in het veld is
**
** parameters:			
**
** Returned value:		int : 	0x00 card found
**								0x01 wrong card found
**								0x02 Carderror (N.U.)
**								0x03 no cards found
**								0x10 error
**								0x20 Request error
**								0x30 CascSel error
** 
*****************************************************************************/
	#define PICC_REQIDL        0x26         //!< request idle
	#define PICC_REQALL        0x52         //!< request all
	#define PICC_ANTICOLL1     0x93         //!< anticollision level 1 106 kBaud
	#define PICC_ANTICOLL11    0x92         //!< anticollision level 1 212 kBaud
	#define PICC_ANTICOLL12    0x94         //!< anticollision level 1 424 kBaud
	#define PICC_ANTICOLL13    0x98         //!< anticollision level 1 848 kBaud
	#define PICC_ANTICOLL2     0x95         //!< anticollision level 2
	#define PICC_ANTICOLL3     0x97         //!< anticollision level 3
	#define PICC_AUTHENT1A     0x60         //!< authentication using key A
	#define PICC_AUTHENT1B     0x61         //!< authentication using key B
	#define PICC_READ16        0x30         //!< read 16 byte block
	#define PICC_WRITE16       0xA0         //!< write 16 byte block
	#define PICC_WRITE4        0xA2         //!< write 4 byte block
	#define PICC_DECREMENT     0xC0         //!< decrement value
	#define PICC_INCREMENT     0xC1         //!< increment value
	#define PICC_RESTORE       0xC2         //!< restore command code
	#define PICC_TRANSFER      0xB0         //!< transfer command code
	#define PICC_HALT          0x50         //!< halt

#define MF_CL1 	0x93
#define MF_CL2 	0x95
#define MF_CL3 	0x97

//MIFARE Kaartlees errors voor getsector en setsector
#define CHECKCARD_OKE				0x00
#define CHECKCARD_WRONGCARD			0x01000
#define CHECKCARD_NOCARD			0x03000
#define CHECKCARD_ERROR				0x10000
#define CHECKCARD_ERROR_REQUEST		0x20000
#define CHECKCARD_ERROR_ANTICOL		0x30000
#define CHECKCARD_ERROR_CASCSEL		0x40000

//MIFARE Kaartlees errors voor getsector en setsector
#define CARDERROR_CARDID_PART1			0x00008000
#define CARDERROR_CARDID_PART2			0x00004000
#define CARDERROR_SALDO_PART1			0x00002000
#define CARDERROR_DOORLOCK_PART1		0x00001000
#define CARDERROR_DOORLOCK_PART2		0x00000800
#define CARDERROR_GOUPLOGITEMS_PART1	0x00000400
#define CARDERROR_OJMAR_PART1			0x00000200
#define CARDERROR_ZONE_ALG_CID			0x00000100
#define CARDERROR_ALL_SECTOR			0xFFFFFFFF

//Algemene kaartfouten
#define CARDERROR_NO_SECTOR_DEFINED		0x00000080
#define CARDERROR_WRONG_MODE			0x00000040
//
#define CARDERROR_NOSECTOR				0x00000020
#define CARDERROR_NOCARD				0x00000010
#define CARDERROR_ADDRESS				0x00000001
#define CARDERROR_AUTH					0x00000002
#define CARDERROR_WRITE					0x00000004
#define CARDERROR_READ					0x00000008
#define CARDERROR_ALL					0x000000FF

uint16_t PN512::CheckCardLogics(/*card_struct *card_data*/)
{	
	uint8_t atq[2];
	uint8_t sak = 0;
	uint8_t uid[11];
	uint16_t resultaat = 0;
	uint32_t hex_num=0;
	RFoff();							
	memset(atq,0x00,2);	
	
	//memset(card_data->uid,0x00,10);	
	
	memset(uid,0x00,11);
	resultaat = Request(PICC_REQALL,&atq[0]); // request for a card	REQ_ALL
	if (resultaat == 0)
	{	if(atq[0] != 0)	//atq = Answer to Request
		{
			resultaat = AntiColl(MF_CL1,uid);	//uid van de kaart opvragen met behulp van anticoll	
			if (resultaat == 0)
			{
				resultaat = Select(MF_CL1,uid,&sak);	// select card met behulp van UID
				if(resultaat == 0)		
				{	
					uid[4] = 0;	
					//******************************************************
					//*** 
					//*** 7 en 10 bytes UID afhandeling
					//*** 
					//*** AN10927
					//*** 
					//******************************************************					
					if (uid[0] == 0x88) //start byte voor langere UID
					{
						uid[0] = uid[1];
						uid[1] = uid[2];
						uid[2] = uid[3];
						uid[3] = 0;
						resultaat = AntiColl(MF_CL2,&uid[3]);	//uid van de kaart opvragen met behulp van anticoll	
						if (resultaat == 0)
						{							
							resultaat = Select(MF_CL2,&uid[3],&sak);	// select card met behulp van UID
							if(resultaat == 0)		
							{				
								uid[7] = 0;									
								if (uid[3] == 0x88) //start byte voor langere UID
								{
									//Bij langere UIDs is UID[0] altijd de manufactor ID welke te vinden is op:
									//http://www.kartenbezogene-identifier.de/de/chiphersteller-kennungen.html
									uid[3] = uid[4];
									uid[4] = uid[5];
									uid[5] = uid[6];
									uid[6] = 0;
									resultaat =AntiColl(MF_CL3,&uid[6]);	//uid van de kaart opvragen met behulp van anticoll	
									if (resultaat == 0)
									{
										resultaat = Select(MF_CL3,&uid[6],&sak);	// select card met behulp van UID
										if(resultaat == 0)		
										{	
											resultaat = MF_HandleSAK(sak);	//10 bytes UID
										} else 
										{	
											resultaat |= CHECKCARD_ERROR_CASCSEL;
											//debugOutput(DEBUG_MIFARE,"PiccCascSel 3 fail",18,resultaat);   
										}
									} else
									{	
										resultaat |= CHECKCARD_ERROR_ANTICOL;
										//debugOutput(DEBUG_MIFARE,"Anticoll 3 fail",18,resultaat);   
									}	
								} else
								{
									resultaat = MF_HandleSAK(sak);	//7 bytes UID
								}
							} else 
							{	
								resultaat |= CHECKCARD_ERROR_CASCSEL;
								//debugOutput(DEBUG_MIFARE,"PiccCascSel 2 fail",18,resultaat);   
							}
						} else
						{	
							resultaat |= CHECKCARD_ERROR_ANTICOL;
							//debugOutput(DEBUG_MIFARE,"Anticoll 2 fail",18,resultaat);   
						}	
					} else
					{
						resultaat = MF_HandleSAK(sak);	//4 bytes UID
					}
				} else 
				{	resultaat |= CHECKCARD_ERROR_CASCSEL;
					//debugOutput(DEBUG_MIFARE,"PiccCascSel failed",18,resultaat);   
				}
			} else
			{	resultaat |= CHECKCARD_ERROR_ANTICOL;
				//debugOutput(DEBUG_MIFARE,"Anticoll failed",18,resultaat);   
			}
		} else
		{	resultaat |= CHECKCARD_ERROR_REQUEST;
			//debugOutput(DEBUG_MIFARE,"PiccRequest failed",18,resultaat); 
		}
	} else
	{	if (resultaat == 0xFF)
		{	resultaat = CHECKCARD_NOCARD;						
		} else
		{	resultaat |= CHECKCARD_ERROR_REQUEST;
			//debugOutput(DEBUG_MIFARE,"PiccRequest failed",18,resultaat); 
		}
	}
	if (resultaat == CHECKCARD_OKE) 
	{
	//	memcpy(card_data->uid,uid,10);
		hex_num = uid[0] << 24;
		hex_num += uid[1] << 16;
		hex_num += uid[2] << 8;
		hex_num += uid[3];
		ESP_LOGI(TAG, "UID = %ld", hex_num);
		resultaat = hex_num & 0xFFFF;   // this is dirty !! Just check it works
//		if ((onCardChange != NULL) && (useEvents == 1)) onCardChange(OKE,card_data->madKeys);				
		return resultaat;
	} else
	{
		RFoff(); // rf field off	 als we niks met de kaart kunnen
//		if ((onCardChange != NULL) && (useEvents == 1)) onCardChange(NOCARD,card_data->madKeys);		
		return resultaat;		// Geen geldige kaart gevonden of errors
	}																					// Geen geldige kaart gevonden of errors
}

uint16_t PN512::MF_HandleSAK(uint8_t sak)
{
	//int derror = 0;
  // http://www.nxp.com/documents/application_note/130830.pdf blz 7
	if ((sak & 0x2) > 0) 									//sak bit 2
	{} 																		//reserved for future use (RFU)
	else
	{
		if ((sak & 0x8) > 0) 								//sak bit 4
	 	{if ((sak & 0x10) > 0) 						//sak bit 5
			{																//mifare 4k card
				return CHECKCARD_OKE;
			} 
			else
			{
				if ((sak & 0x1) > 0) 						//sak bit 1
				{																//mifare mini															
					return CHECKCARD_OKE;
				} 
				else
				{																//mifare 1k																
					return CHECKCARD_OKE;
				}
			}
		} 
//		else
//		{ if ((sak & 0x10) > 0) 						//sak bit 5
//			{if ((sak & 0x1) > 0) 						//sak bit 1
//				{} 															//mifare plus 4k																
//				else
//				{}															//mifare plus 2k
//			}else
//			{
//			  if ((sak & 0x20) > 0) 					//sak bit 6
//				{
//					unsigned char rats[] = { 0xE0, 
//											 0x50 };  				// The PN512 can has a buffer of 64 bytes and we set the cid to 0
//					unsigned char ats[64];				// PN512 buffer size bigger is not needed
//					memset(ats,0x00,50);   				// Just to easy up debugging, is not needed
//																				//mifare desfire we need to RATS = request ATS
//					derror = Mifare_HW_Rats(SPIBus,DeviceNmr,rats,ats);
//					if (derror == 0)
//					{
//						// ats[0] = length																		
//						// ats[1] = format type ( if (ats[0] > 1) then the format type is present )
//						//				- b8 		= 0x00 RFU
//						//				- b7 		= interface byte T(a) present
//						//				- b6		= interface byte T(b) present
//						//				- b5		= interface byte T(c) present
//						//				- b4 .. b1 	= buffer size of the card
//						// ats[2] = T(a) 
//						//				- b8 		= Same divisors support for different directions 0 = no; 1 = yes
//						//				- b7 ..	B5	= bitrate from card to PN512
//						//					* b6		= DS=8 supported								
//						//					* b6		= DS=4 supported
//						//					* b5		= DS=2 supported
//						//				- b4 		= 0x00 RFU
//						//				- b3 .. B1  = bitrate support from PN512 to card
//						//					* b3		= DR=8 supported
//						//					* b2		= DR=4 supported
//						//					* b1 		= DR=2 supported
//						// ats[3] = T(b) 
//						//				- b8 .. b5	= SWT  wait time: the time the card needs before it can reply on received data.
//						//				- b4 .. b1	= SFGI The time the card needs before it can receive data after sending data. calcultated as: (256 x 16 / fc) x 2^SFGI
//						// ats[4] = T(c)
//						//				- b8 .. b3  = 0x00 RFU
//						//				- b2		= CID supported
//						//				- b1		= NAD supported
//						// ats[5] .. rest = historical bytes ISO/IEC 7816-4
//						if (ats[0] > 1)
//						{
//							if ((ats[1] & 0x40) > 0)	//T(a) present?
//							{
//								
//								if (ats[2] > 0) //can we choose the data rates?
//								{												
//									//PPSS request
////									unsigned char ppss[] = { 	0xD0, 	//CID is set to 0
////																0x11,
////																0x00	//for now we set the data rate to 1 as they are always supported
////															 };
//																					
//								}
//							}
//						}
//					}
//				} else
//				{
//				 	//mifare UL card (nexus_s for nfc)

//				}					 	
//			}
//		}
	}
	return CHECKCARD_ERROR;
}