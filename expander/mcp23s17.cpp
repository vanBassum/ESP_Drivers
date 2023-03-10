#include "mcp23s17.h"
#include <string.h>
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "MCP23S17";
#define MCP23S17_MANUF_CHIP_ADDRESS	0x40	// Default Address

typedef enum
{
	//  REGISTERS                      //  description                   datasheet
	MCP23S17_REG_DIR_A         = 0x00, //  Data Direction Register A     P18
	MCP23S17_REG_DIR_B         = 0x01, //  Data Direction Register B     P18
	MCP23S17_REG_POL_A         = 0x02, //  Input Polarity A              P18
	MCP23S17_REG_POL_B         = 0x03, //  Input Polarity B              P18
	MCP23S17_REG_GPINTEN_A     = 0x04, //  NOT USED interrupt enable     P19
	MCP23S17_REG_GPINTEN_B     = 0x05, //  NOT USED                      
	MCP23S17_REG_DEFVAL_A      = 0x06, //  NOT USED interrupt def        P19
	MCP23S17_REG_DEFVAL_B      = 0x07, //  NOT USED                      
	MCP23S17_REG_INTCON_A      = 0x08, //  NOT USED interrupt control    P20
	MCP23S17_REG_INTCON_B      = 0x09, //  NOT USED                      
	MCP23S17_REG_IOCON_A       = 0x0A, //  IO control register           P20
	MCP23S17_REG_IOCON_B       = 0x0B, //  NOT USED                      
	MCP23S17_REG_PUR_A         = 0x0C, //  Pull Up Resistors A           P22
	MCP23S17_REG_PUR_B         = 0x0D, //  Pull Up Resistors A           P22
	MCP23S17_REG_INTF_A        = 0x0E, //  NOT USED interrupt flag       P22
	MCP23S17_REG_INTF_B        = 0x0F, //  NOT USED                      
	MCP23S17_REG_INTCAP_A      = 0x10, //  NOT USED interrupt capture    P23
	MCP23S17_REG_INTCAP_B      = 0x11, //  NOT USED                      
	MCP23S17_REG_GPIO_A        = 0x12, //  General Purpose IO A          P23
	MCP23S17_REG_GPIO_B        = 0x13, //  General Purpose IO B          P23
	MCP23S17_REG_OLAT_A        = 0x14, //  NOT USED output latch         P24
	MCP23S17_REG_OLAT_B        = 0x15, //  NOT USED
}mcp23s17_reg_t;

typedef enum
{
	//  IOCR = IO CONTROL REGISTER bit masks   - details datasheet P20
	MCP23S17_IOCR_BANK		   = 0x80,   //  Controls how the registers are addressed.
	MCP23S17_IOCR_MIRROR	   = 0x40,   //  INT Pins Mirror bit.
	MCP23S17_IOCR_SEQOP		   = 0x20,   //  Sequential Operation mode bit.
	MCP23S17_IOCR_DISSLW	   = 0x10,   //  Slew Rate control bit for SDA output.
	MCP23S17_IOCR_HAEN		   = 0x08,   //  Hardware Address Enable bit (MCP23S17 only).
	MCP23S17_IOCR_ODR		   = 0x04,   //  Configures the INT pin as an open-drain output.
	MCP23S17_IOCR_INTPOL	   = 0x02,   //  This bit sets the polarity of the INT output pin.
	MCP23S17_IOCR_NI		   = 0x01,   //  Not implemented.
}mcp23s17_iocr_t;



esp_err_t ESP_Drivers::MCP23S17::Init(SPIBus* spiBus, gpio_num_t cs, gpio_num_t irq, transaction_cb_t pre_cb, transaction_cb_t post_cb)
{
	irqPin = irq;
	
	esp_err_t result;
	spi_device_interface_config_t spi_devcfg;
	memset(&spi_devcfg, 0, sizeof(spi_devcfg));
	spi_devcfg.address_bits = 0;
	spi_devcfg.command_bits = 0;
	spi_devcfg.dummy_bits = 0;
	spi_devcfg.mode = 0;
	spi_devcfg.duty_cycle_pos = 0;
	spi_devcfg.cs_ena_posttrans = 0;
	spi_devcfg.cs_ena_pretrans = 0;
	spi_devcfg.clock_speed_hz = 9 * 1000 * 1000;			//@10MHz the glcd shows artifacts.
	spi_devcfg.flags = 0;
	spi_devcfg.queue_size = 7;
	spi_devcfg.spics_io_num = cs;
	spi_devcfg.pre_cb = pre_cb;
	spi_devcfg.post_cb = post_cb;
	result = spidev.Init(spiBus, &spi_devcfg);	
	
	spidev.AcquireBus();
	Write8(MCP23S17_REG_IOCON_A, 0x08);    					// Set up ICON A,B to auto increment
	spidev.ReleaseBus();
	
	
	
	return result;
}


esp_err_t ESP_Drivers::MCP23S17::Transmit(uint8_t * txData, uint8_t * rxData, uint8_t count)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       			
	t.length = (count * 8);              		
	t.tx_buffer = txData;               		
	t.rx_buffer = rxData;
	return spidev.PollingTransmit(&t);  		
}

uint8_t ESP_Drivers::MCP23S17::Read8(uint8_t reg)
{
	uint8_t txData[3];
	uint8_t rxData[3];	
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1) | 0x01;
	txData[1] = reg;
	txData[2] = 0x00;
	Transmit(txData, rxData, 3);
	return rxData[2];
}

void ESP_Drivers::MCP23S17::Write8(uint8_t reg, uint8_t value)
{
	uint8_t txData[3];
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1);
	txData[1] = reg;
	txData[2] = value;
	Transmit(txData, NULL, 3);
}

	
uint16_t ESP_Drivers::MCP23S17::Read16(uint8_t reg)
{
	uint8_t txData[4];
	uint8_t rxData[4];	
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1) | 0x01;
	txData[1] = reg;
	txData[2] = 0x00;
	txData[3] = 0x00;
	Transmit(txData, rxData, 4);
	return (rxData[3] << 8) | rxData[2];
}

void ESP_Drivers::MCP23S17::Write16(uint8_t reg, uint16_t value)
{
	uint8_t txData[4];
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1);
	txData[1] = reg;
	txData[2] = (uint8_t)(value);
	txData[3] = (uint8_t)(value >> 8);
	Transmit(txData, NULL, 4);
}




void ESP_Drivers::MCP23S17::SetPins(ESP_Drivers::mcp23s17_pins_t mask, ESP_Drivers::mcp23s17_pins_t value)
{
	mutex.Take();
	pinBuffer = (pinBuffer & ~mask) | (value & mask);
	
	spidev.AcquireBus();
	Write16(MCP23S17_REG_GPIO_A, pinBuffer);
	spidev.ReleaseBus();
	mutex.Give();
}


ESP_Drivers::mcp23s17_pins_t ESP_Drivers::MCP23S17::GetPins(ESP_Drivers::mcp23s17_pins_t mask)
{
	mutex.Take();
	spidev.AcquireBus();
	mcp23s17_pins_t pins = (mcp23s17_pins_t)Read16(MCP23S17_REG_GPIO_A);
	spidev.ReleaseBus();
	mutex.Give();
	return pins & mask;
}


void ESP_Drivers::MCP23S17::SetPinsMode(ESP_Drivers::mcp23s17_pins_t mask, ESP_Drivers::mcp23s17_pinmodes_t mode)
{
	mutex.Take();
	if (mode == MCP23S17_PINMODE_INPUT)
		pinDirBuffer = (pinDirBuffer & ~mask) | (MCP23S17_PIN_ALL & mask);
	else
		pinDirBuffer = pinDirBuffer & ~mask;
		
	spidev.AcquireBus();
	Write16(MCP23S17_REG_DIR_A, pinDirBuffer);
	spidev.ReleaseBus();
	mutex.Give();
}

void ESP_Drivers::MCP23S17::ConsecutivePinWriting(ESP_Drivers::mcp23s17_pins_t mask, ESP_Drivers::mcp23s17_pins_t* values, size_t size)
{
	mutex.Take();
	size_t totSize = 2 + size * 2;
	uint8_t txData[totSize];
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1);
	txData[1] = MCP23S17_REG_GPIO_A;
	
	for (int i = 0; i < size; i++)
	{
		pinBuffer = (pinBuffer & ~mask) | (values[i] & mask);
		txData[i * 2 + 2] = (uint8_t)pinBuffer;
		txData[i * 2 + 3] = (uint8_t)(pinBuffer>>8);
	}
	spidev.AcquireBus();
	Write8(MCP23S17_REG_IOCON_A, 0x28);	//seqop = BIT 5 0X20
	Transmit(txData, NULL, totSize);
	Write8(MCP23S17_REG_IOCON_A, 0x08);	//seqop = BIT 5 0X20
	spidev.ReleaseBus();
	mutex.Give();
}
