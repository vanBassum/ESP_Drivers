#include "pn512.h"
#include "task.h"

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
