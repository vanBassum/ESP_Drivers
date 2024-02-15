#include "mcp23s17.h"


//static const char *TAG = "MCP23S17";
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


DeviceResult MCP23S17::setDeviceConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiDevice", &spiDeviceKey),  DeviceStatus::ConfigError, DeviceResult::ConfigError, TAG, "Missing parameter: spiDevice");
	setStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult MCP23S17::loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(spiDevice, deviceManager->getDeviceByKey<ISpiDevice>(spiDeviceKey), DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "Dependencies not ready %d", (int)getDeviceStatus());
	setStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}

DeviceResult MCP23S17::init()
{
	ContextLock lock(mutex);
	Write8(MCP23S17_REG_IOCON_A, 0x08);    					// Set up ICON A,B to auto increment
	setStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult MCP23S17::portConfigure(uint32_t port, uint8_t mask, GpioFlags flags) 
{
	if(flags & GpioFlags::GPIO_FLAGS_INPUT)
		pinDirBuffer[port] = (pinDirBuffer[port] & ~mask) | mask;
	if(flags & GpioFlags::GPIO_FLAGS_OUTPUT)
		pinDirBuffer[port] = pinDirBuffer[port] & ~mask;
	
	uint16_t val = pinDirBuffer[1] << 8 | pinDirBuffer[0];
	return Write16(MCP23S17_REG_DIR_A, val);
}

DeviceResult MCP23S17::portRead(uint32_t port, uint8_t mask, uint8_t* value) 		 
{
   	ContextLock lock(mutex);
	uint16_t val;
	DeviceResult result = Read16(MCP23S17_REG_GPIO_A, &val);
	*value = ((uint8_t*)&val)[port];
	return result;
}

DeviceResult MCP23S17::portWrite(uint32_t port, uint8_t mask, uint8_t value) 		 
{
	ContextLock lock(mutex);
	pinBuffer[port] = (pinBuffer[port] & ~mask) | (value & mask);
	uint16_t val = pinBuffer[1] << 8 | pinBuffer[0];
	return Write16(MCP23S17_REG_GPIO_A, val);
}

DeviceResult MCP23S17::Transmit(uint8_t *txData, uint8_t *rxData, uint8_t count)
{
	//TODO: Claim the bus!
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiDevice->Transmit(txData, rxData, count) == DeviceResult::Ok, DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "spiDevice->Transmit returned error");
    return DeviceResult::Ok;
}

DeviceResult MCP23S17::Write8(uint8_t reg, uint8_t value)
{
	uint8_t txData[3];
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1);
	txData[1] = reg;
	txData[2] = value;
	return Transmit(txData, NULL, 3);
}

DeviceResult MCP23S17::Read8(uint8_t reg, uint8_t *value)
{
	uint8_t txData[3];
	uint8_t rxData[3];	
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1) | 0x01;
	txData[1] = reg;
	txData[2] = 0x00;
	DeviceResult result = Transmit(txData, rxData, 3);
	*value = rxData[2];
    return result;
}

DeviceResult MCP23S17::Read16(uint8_t reg, uint16_t *value)
{
	uint8_t txData[4];
	uint8_t rxData[4];	
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1) | 0x01;
	txData[1] = reg;
	txData[2] = 0x00;
	txData[3] = 0x00;
	DeviceResult result = Transmit(txData, rxData, 4);
	*value = (rxData[3] << 8) | rxData[2];
	return result;
}

DeviceResult MCP23S17::Write16(uint8_t reg, uint16_t value)
{
	uint8_t txData[4];
	txData[0] = MCP23S17_MANUF_CHIP_ADDRESS | (devAddr << 1);
	txData[1] = reg;
	txData[2] = (uint8_t)(value);
	txData[3] = (uint8_t)(value >> 8);
	return Transmit(txData, NULL, 4);
}

