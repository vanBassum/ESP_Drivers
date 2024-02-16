#include "MAX14830.h"
#include "max310x.h"

#define MAX14830_BRGCFG_CLKDIS_BIT		(1 << 6) /* Clock Disable */
#define MAX14830_REV_ID					(0xb0)
#define MAX14830_USE_XTAL
#define MAX14830_CLK					4000000
#define MAX14830_FIFO_MAX				128

static const GpioConfig isrDisabledConfig = CREATE_GPIO_CONFIG(GpioMode::GPIO_CFG_MODE_INPUT, GpioIntr::GPIO_CFG_INTR_DISABLE, GpioPullFlags::GPIO_CFG_PULL_DISABLE);
static const GpioConfig isrEnabledConfig  = CREATE_GPIO_CONFIG(GpioMode::GPIO_CFG_MODE_INPUT, GpioIntr::GPIO_CFG_INTR_LOW_LEVEL, GpioPullFlags::GPIO_CFG_PULL_DISABLE);

DeviceResult MAX14830::setDeviceConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiDevice", &spiDeviceKey),  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Missing parameter: spiDevice");

    const char *isrPinStr = nullptr;
    DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("isrPin", &isrPinStr),  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Missing parameter: isrPin");
    DEV_SET_STATUS_AND_RETURN_ON_FALSE(sscanf(isrPinStr, "%m[^,],%hhu,%hhu", &isrDeviceKey, &isrPort, &isrPin) == 3,  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Error parsing isrPin");

	setStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult MAX14830::loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(spiDevice, deviceManager->getDeviceByKey<SpiDevice>(spiDeviceKey), DeviceStatus::Dependencies, DeviceResult::Error, TAG, "Missing dependency: spiDevice");
	GET_DEV_OR_RETURN(isrDevice, deviceManager->getDeviceByKey<IGpio>(isrDeviceKey), DeviceStatus::Dependencies, DeviceResult::Error, TAG, "Missing dependency: isrDevice");
	setStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}

DeviceResult MAX14830::init()
{
	ContextLock lock(mutex);
	
    // Detect and setup clock
    bool detected;
    uint32_t clk;
    bool clkError;
    DEV_RETURN_ON_FALSE(Detect(&detected) == DeviceResult::Ok, DeviceResult::Error, TAG, "Error while detecting chip");
    DEV_RETURN_ON_FALSE(detected, DeviceResult::Error, TAG, "Chip not detected");
    DEV_RETURN_ON_FALSE(SetRefClock(&clk, &clkError) == DeviceResult::Ok, DeviceResult::Error, TAG, "Error while setting reference clock");
    DEV_RETURN_ON_FALSE(clkError, DeviceResult::Error, TAG, "Clock error");

	DEV_RETURN_ON_ERROR(portInit(0x00), TAG, "Error while port 0 init");
	DEV_RETURN_ON_ERROR(portInit(0x01), TAG, "Error while port 1 init");
	DEV_RETURN_ON_ERROR(portInit(0x02), TAG, "Error while port 2 init");
	DEV_RETURN_ON_ERROR(portInit(0x03), TAG, "Error while port 3 init");

    // Enable the ISR handling
	DEV_RETURN_ON_ERROR_SILENT(isrDevice->portIsrAddCallback(isrPort, isrPin, [&](){isr_handler();}));
	DEV_RETURN_ON_ERROR_SILENT(isrDevice->portConfigure(isrPort, 1<<isrPin, &isrEnabledConfig));

	// Tell the driver the device is initialized and ready to use.
	setStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult MAX14830::Detect(bool* result)
{
    uint8_t value;
    DEV_RETURN_ON_ERROR_SILENT(regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_ENBL));
    DEV_RETURN_ON_ERROR_SILENT(max310x_port_read(0x00, MAX310X_REVID_EXTREG, &value));
    DEV_RETURN_ON_ERROR_SILENT(regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_DSBL));

    if (((value & MAX310x_REV_MASK) != MAX14830_REV_ID))
	{
		ESP_LOGE(TAG, "Chip not found revid %d", value);
        *result = false;
		return DeviceResult::Ok;
	}
    *result = true;
    return DeviceResult::Ok;
}

DeviceResult MAX14830::SetRefClock(uint32_t* clk, bool* clkError)
{
    uint32_t div, clksrc, pllcfg = 0;
	int64_t besterr = -1;
	uint64_t fdiv, fmul, bestfreq = MAX14830_CLK;

	// First, update error without PLL
	max310x_update_best_err(MAX14830_CLK, &besterr);

	// Try all possible PLL dividers
	for (div = 1; (div <= 63) && besterr; div++)
	{
		fdiv = MAX14830_CLK / div;

		/* Try multiplier 6 */
		if ((fdiv >= 500000) && (fdiv <= 800000))
		{
			fmul = fdiv * 6;
			if (!max310x_update_best_err(fmul, &besterr))
			{
				pllcfg = (0 << 6) | div;
				bestfreq = fmul;
			}
		}

		/* Try multiplier 48 */
		if ((fdiv >= 850000) && (fdiv <= 1200000))
		{
			fmul = fdiv * 48;
			if (!max310x_update_best_err(fmul, &besterr))
			{
				pllcfg = (1 << 6) | div;
				bestfreq = fmul;
			}
		}

		/* Try multiplier 96 */
		if ((fdiv >= 425000) && (fdiv <= 1000000))
		{
			fmul = fdiv * 96;
			if (!max310x_update_best_err(fmul, &besterr))
			{
				pllcfg = (2 << 6) | div;
				bestfreq = fmul;
			}
		}

		/* Try multiplier 144 */
		if ((fdiv >= 390000) && (fdiv <= 667000))
		{
			fmul = fdiv * 144;
			if (!max310x_update_best_err(fmul, &besterr))
			{
				pllcfg = (3 << 6) | div;
				bestfreq = fmul;
			}
		}
	}

	/* Configure clock source */
#ifdef MAX14830_USE_XTAL
	clksrc = MAX310X_CLKSRC_CRYST_BIT;
#else
	clksrc = MAX310X_CLKSRC_EXTCLK_BIT;
#endif

	/* Configure PLL */
	if (pllcfg)
	{
		clksrc |= MAX310X_CLKSRC_PLL_BIT;
		regmap_write(MAX310X_PLLCFG_REG, pllcfg);
	}
	else
	{
		clksrc |= MAX310X_CLKSRC_PLLBYP_BIT;
	}
	regmap_write(MAX310X_CLKSRC_REG, clksrc);

#ifdef MAX14830_USE_XTAL
	/* Wait for crystal */
	uint8_t val;
	uint8_t escape = 0;
	while (escape < 100)
	{
		regmap_read(MAX310X_STS_IRQSTS_REG, &val);
		if (!(val & MAX310X_STS_CLKREADY_BIT))
		{
			vTaskDelay(pdMS_TO_TICKS(20));
			//ESP_LOGE(TAG, "Clock is not stable yet");
		}
		else
		{
			escape = 255;
		}
	}
	if (escape != 255)
	{
		ESP_LOGE(TAG, "Clock not stable");
		*clkError = true;
	}
#endif
    *clk = (uint32_t)bestfreq;
    return DeviceResult::Ok;
}

DeviceResult MAX14830::Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t * cmdData, uint8_t count)
{
    spi_transaction_t t = {0};
	memset(&t, 0, sizeof(t));       				    //Zero out the transaction
	t.length = (count * 8);                   		    //amount of bits
	t.tx_buffer = cmdData;               			    //The data is the cmd itself
	t.cmd = 0x80 | cmd;								    //Add write bit
    DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiDevice->Transmit(&t, SPIFlags::POLLED) == DeviceResult::Ok, DeviceStatus::Dependencies, DeviceResult::Error, TAG, "Error in spi transmit");
	return DeviceResult::Ok;
}

DeviceResult MAX14830::Max14830_ReadBufferPolled(uint8_t cmd, uint8_t * cmdData, uint8_t * replyData, uint8_t count)
{
	spi_transaction_t t = {0};
	memset(&t, 0, sizeof(t));       				//Zero out the transaction
	t.length = (count * 8);              			//amount of bits
	t.tx_buffer = cmdData;               			//The data is the cmd itself
	t.rx_buffer = replyData;
	t.cmd = cmd;
    DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiDevice->Transmit(&t, SPIFlags::POLLED) == DeviceResult::Ok, DeviceStatus::Dependencies, DeviceResult::Error, TAG, "Error in spi transmit");
	return DeviceResult::Ok;
}



DeviceResult MAX14830::regmap_write(uint8_t cmd, uint8_t value)
{
    return Max14830_WriteBufferPolled(cmd, &value, 1);
}

DeviceResult MAX14830::regmap_read(uint8_t cmd, uint8_t * value)
{
	uint8_t cmdData[1];
	return Max14830_ReadBufferPolled(cmd, cmdData, value, 1);
}

DeviceResult MAX14830::max310x_port_read(uint8_t port, uint8_t cmd, uint8_t* value)
{
	cmd = ((uint32_t)port << 5) | cmd;
	return regmap_read(cmd, value);
}

DeviceResult MAX14830::max310x_port_write(uint8_t port, uint8_t cmd, uint8_t value)
{
	cmd = ((uint32_t)port << 5) | cmd;
	return regmap_write(cmd, value);
}

uint8_t MAX14830::max310x_update_best_err(uint64_t f, int64_t * besterr)
{
	/* Use baudrate 115200 for calculate error */
	int64_t err = f % (115200 * 16);
	if ((*besterr < 0) || (*besterr > err))
	{
		*besterr = err;
		return 0;
	}
	return 1;
}

void MAX14830::isr_handler()
{
	//TODO: How do we ensure that this code is callable from ISR?
	//Or we dont call this from the ISR, and work with detecting flanks.
	isrDevice->portConfigure(isrPort, isrPin, &isrDisabledConfig);	//Disable isr, until task has handled the work.

	xTimerPendFunctionCallFromISR(processIsr, this, 0, NULL);
}

void MAX14830::processIsr(void * pvParameter1, uint32_t ulParameter2)
{
	MAX14830* device = (MAX14830*)pvParameter1;
	ContextLock lock(device->mutex);

	//TODO: We should check the returns of this!!!
	device->handleIRQForPort(0x00); 
	device->handleIRQForPort(0x01); 
	device->handleIRQForPort(0x02); 
	device->handleIRQForPort(0x03); 	

	//All reasons for ISR should be gone now, so re-enable the isr pin.
	//TODO: We should check the returns of this!!!
	device->isrDevice->portConfigure(device->isrPort, device->isrPin, &isrEnabledConfig);
}

DeviceResult MAX14830::handleIRQForPort(uint8_t port)
{
	ESP_LOGI(TAG, "ISR");
	uint8_t isrReg;
	DEV_RETURN_ON_ERROR_SILENT(max310x_port_read(port, MAX310X_IRQSTS_REG, &isrReg));		
	bool hasData = HAS_BIT(isrReg, MAX310X_IRQ_RXEMPTY_BIT); //inverted in the config
	//TODO: Do something with the knowledge that there is data avaiable

	if (HAS_BIT(isrReg, MAX310X_IRQ_STS_BIT))
	{
		uint8_t sts;
		DEV_RETURN_ON_ERROR_SILENT(max310x_port_read(port, MAX310X_STS_IRQSTS_REG, &sts));
		//TODO: Do something with the knowledge that there are pinchanges.
	}
	return DeviceResult::Ok;
}

DeviceResult MAX14830::portInit(uint8_t port)
{
	// Clear IRQ status registers
	uint8_t dummy;
	DEV_RETURN_ON_ERROR_SILENT(max310x_port_read(port, MAX310X_STS_IRQSTS_REG, &dummy));
	DEV_RETURN_ON_ERROR_SILENT(max310x_port_read(port, MAX310X_GLOBALIRQ_REG, &dummy));

	// Enable IO IRQ
	DEV_RETURN_ON_ERROR_SILENT(max310x_port_write(port, MAX310X_IRQEN_REG, MAX310X_IRQ_STS_BIT));
	return DeviceResult::Ok;
}

DeviceResult MAX14830::SetPinsMode(uint8_t port, uint8_t mask, GpioMode mode)
{

	uint8_t minimask = mask & 0xF;
	if (minimask > 0)
	{
		uint8_t od = (gpioConfBuffer[port] >> 4) & 0xf;
		uint8_t ou = (gpioConfBuffer[port]) & 0xf;
		switch (mode)
		{
		case GPIO_CFG_MODE_INPUT:
			ou &= ~minimask;
			break;
		case GPIO_CFG_MODE_OUTPUT:
			ou |= minimask;
			od &= ~minimask;
			break;
		case GPIO_CFG_MODE_OUTPUT_OD:
			ou |= minimask;
			od |= minimask;
			break;
		default:
			return DeviceResult::NotSupported;
		}
		
		gpioConfBuffer[port] = (od << 4) | ou;
		return max310x_port_write(port, MAX310X_GPIOCFG_REG, gpioConfBuffer[port]);
	}
	return DeviceResult::Ok;
}

DeviceResult MAX14830::SetPins(uint8_t port, uint8_t mask, uint8_t value)
{
	ContextLock lock(mutex);
	uint8_t minimask = mask & 0xF;
	uint8_t minivalue = value & 0xF;
	if (minimask > 0)
	{
		uint8_t ou = (gpioDataBuffer[port]) & 0xf;
		ou &= ~minimask;
		ou |= minivalue & minimask;
		gpioDataBuffer[port] = ou;
		return max310x_port_write(port, MAX310X_GPIODATA_REG, gpioDataBuffer[port]);
	}
	return DeviceResult::Ok;
}

DeviceResult MAX14830::GetPins(uint8_t port, uint8_t mask, uint8_t* value)
{
	ContextLock lock(mutex);
	uint8_t minimask = mask & 0xF;
	if (minimask > 0)
	{
		uint8_t reg;
		DEV_RETURN_ON_ERROR_SILENT(max310x_port_read(port, MAX310X_GPIODATA_REG, &reg));
		*value = reg & minimask;
	}
	return DeviceResult::Ok;
}
