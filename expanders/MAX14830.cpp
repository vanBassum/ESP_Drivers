#include "MAX14830.h"
#include "max310x.h"

#define MAX14830_BRGCFG_CLKDIS_BIT		(1 << 6) /* Clock Disable */
#define MAX14830_REV_ID					(0xb0)
#define MAX14830_USE_XTAL
#define MAX14830_CLK					4000000
#define MAX14830_FIFO_MAX				128

static const GpioConfig isrDisabledConfig = GPIO_CREATE_CONFIG(GpioConfigMode::GPIO_CFG_MODE_INPUT, GpioConfigIntr::GPIO_CFG_INTR_DISABLE, GpioConfigPull::GPIO_CFG_PULL_DISABLE);
static const GpioConfig isrEnabledConfig  = GPIO_CREATE_CONFIG(GpioConfigMode::GPIO_CFG_MODE_INPUT, GpioConfigIntr::GPIO_CFG_INTR_NEGEDGE, GpioConfigPull::GPIO_CFG_PULL_DISABLE);

std::list<std::shared_ptr<MAX14830::IsrHandle>> MAX14830::callbacks; 	//Initialize callbacks

Result MAX14830::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	RETURN_ON_ERR(config.getProperty("spiDevice", &spiDeviceKey));

    const char *isrPinStr = nullptr;
    RETURN_ON_ERR(config.getProperty("isrPin", &isrPinStr));
	int result = sscanf(isrPinStr, "%m[^,],%hhu,%hhu", &isrDeviceKey, &isrPort, &isrPin);
	if(result != 3)
	{
		ESP_LOGE(TAG, "Error parsing isrPin");
		return Result::Error;
	}

	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result MAX14830::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(deviceManager->getDeviceByKey<SpiDevice>(spiDeviceKey, spiDevice));
	RETURN_ON_ERR(deviceManager->getDeviceByKey<IGpio>(isrDeviceKey, isrDevice));
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result MAX14830::DeviceInit()
{
	ContextLock lock(mutex);
	
    // Detect and setup clock
    bool detected;
    uint32_t clk;
    bool clkError;
    RETURN_ON_ERR(Detect(&detected));
	if(!detected)
	{
		ESP_LOGE(TAG, "Chip not detected");
		return Result::Error;
	}

    RETURN_ON_ERR(SetRefClock(&clk, &clkError));
	if(!clkError)
	{
		ESP_LOGE(TAG, "Clock error");
		return Result::Error;
	}


	RETURN_ON_ERR(portInit(0x00));
	RETURN_ON_ERR(portInit(0x01));
	RETURN_ON_ERR(portInit(0x02));
	RETURN_ON_ERR(portInit(0x03));

    // Enable the ISR handling
	RETURN_ON_ERR(isrDevice->GpioIsrAddCallback(isrPort, isrPin, [&](){isr_handler();}));
	RETURN_ON_ERR(isrDevice->GpioConfigure(isrPort, 1<<isrPin, &isrEnabledConfig));

	// Tell the driver the device is initialized and ready to use.
	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
}

Result MAX14830::Detect(bool* result)
{
    uint8_t value;
    RETURN_ON_ERR(regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_ENBL));
    RETURN_ON_ERR(max310x_port_read(0x00, MAX310X_REVID_EXTREG, &value));
    RETURN_ON_ERR(regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_DSBL));

    if (((value & MAX310x_REV_MASK) != MAX14830_REV_ID))
	{
		ESP_LOGE(TAG, "Chip not found revid %d", value);
        *result = false;
		return Result::Ok;
	}
    *result = true;
    return Result::Ok;
}

Result MAX14830::SetRefClock(uint32_t* clk, bool* clkError)
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
    return Result::Ok;
}

Result MAX14830::Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t * cmdData, uint8_t count)
{
    spi_transaction_t t;
	memset(&t, 0, sizeof(t));       				    //Zero out the transaction
	t.length = (count * 8);                   		    //amount of bits
	t.tx_buffer = cmdData;               			    //The data is the cmd itself
	t.cmd = 0x80 | cmd;								    //Add write bit
    RETURN_ON_ERR(spiDevice->Transmit(&t, SPIFlags::POLLED));
	return Result::Ok;
}

Result MAX14830::Max14830_ReadBufferPolled(uint8_t cmd, uint8_t * cmdData, uint8_t * replyData, uint8_t count)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       				//Zero out the transaction
	t.length = (count * 8);              			//amount of bits
	t.tx_buffer = cmdData;               			//The data is the cmd itself
	t.rx_buffer = replyData;
	t.cmd = cmd;
    RETURN_ON_ERR(spiDevice->Transmit(&t, SPIFlags::POLLED));
	return Result::Ok;
}

Result MAX14830::regmap_write(uint8_t cmd, uint8_t value)
{
    return Max14830_WriteBufferPolled(cmd, &value, 1);
}

Result MAX14830::regmap_read(uint8_t cmd, uint8_t * value)
{
	uint8_t cmdData[1];
	return Max14830_ReadBufferPolled(cmd, cmdData, value, 1);
}

Result MAX14830::max310x_port_read(uint8_t port, uint8_t cmd, uint8_t* value)
{
	cmd = (port << 5) | cmd;
	return regmap_read(cmd, value);
}

Result MAX14830::max310x_port_write(uint8_t port, uint8_t cmd, uint8_t value)
{
	cmd = (port << 5) | cmd;
	return regmap_write(cmd, value);
}

Result MAX14830::max310x_port_update(uint8_t port, uint8_t cmd, uint8_t mask, uint8_t value)
{
	uint8_t val = 0;
	RETURN_ON_ERR(max310x_port_read(port, cmd, &val));
	val &= ~mask;
	val |= (mask & value);
	return max310x_port_write(port, cmd, val);
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
	// Switching context, don't do SPI things from the ISR.
	xTimerPendFunctionCallFromISR(processIsr, this, 0, NULL);
}

void MAX14830::processIsr(void * pvParameter1, uint32_t ulParameter2)
{
	MAX14830* device = (MAX14830*)pvParameter1;
	//Do not lock, otherwise the isr callbacks will have problems!

	uint8_t gisr;

	//TODO: We should check the returns of this!!!
	device->regmap_read(MAX310X_GLOBALIRQ_REG, &gisr);
	if((gisr & 0x01) == 0) device->handleIRQForPort(0x00); 
	if((gisr & 0x02) == 0) device->handleIRQForPort(0x01); 
	if((gisr & 0x04) == 0) device->handleIRQForPort(0x02); 
	if((gisr & 0x08) == 0) device->handleIRQForPort(0x03); 	
}

Result MAX14830::handleIRQForPort(uint8_t port)
{
	uint8_t isr;
	uint8_t sts = 0;
	RETURN_ON_ERR(readIsrRegisters(port, &isr, &sts));

	if(HAS_BIT(isr, MAX310X_IRQ_RXEMPTY_BIT))
	{
		uartRxSemaphores[port].Give();
	}

	//TODO: Optimize this code
	for(int bit=0; bit < 4; bit++)	//Only 4 pins
	{
		uint8_t pin = 1<<bit;
		if(sts & pin)
		{
			// Search for the callback associated with the GPIO pin and call the callback
			for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
				if ((*it)->port == port && (*it)->pin == pin) {
					(*it)->callback();
				}
			}
		}
	}

	return Result::Ok;
}

Result MAX14830::readIsrRegisters(uint8_t port, uint8_t* isr, uint8_t* sts)
{
	ContextLock lock(mutex);

	RETURN_ON_ERR(max310x_port_read(port, MAX310X_IRQSTS_REG, isr));		
	bool pinChanges = HAS_BIT(*isr, MAX310X_IRQ_STS_BIT);
	if (pinChanges)
	{
		RETURN_ON_ERR(max310x_port_read(port, MAX310X_STS_IRQSTS_REG, sts));
	}
	return Result::Ok;
}

Result MAX14830::max310x_get_ref_clk(uint32_t* refClk)
{
	uint64_t clk = MAX14830_CLK;	//we only need 64bits for calculation..
	uint8_t value = 0;
	RETURN_ON_ERR(max310x_port_read(0x00, MAX310X_PLLCFG_REG, &value));
	uint32_t clkDiv = value & MAX310X_PLLCFG_PREDIV_MASK;
	uint32_t mul = (value >> 6) & 0x3;
	if (mul == 0)
	{
		mul = 6;
	}
	else
	{
		mul = mul * 48;
	}
	if(refClk != NULL)
		*refClk = (clk * mul) / clkDiv;	//math is turned to prevent math errors (trunc).
	return Result::Ok;
}

Result MAX14830::max310x_set_baud(uint8_t port, uint32_t baud, uint32_t* actualBaud)
{
	uint8_t mode = 0;
	uint32_t fref;
	RETURN_ON_ERR(max310x_get_ref_clk(&fref));
	uint32_t clk = fref;
	uint32_t div = clk / baud;

	// Check for minimal value for divider
	if (div < 16)
		div = 16;
	//
	if ((clk % baud) && ((div / 16) < 0x8000))
	{
		/* Mode x2 */
		mode = MAX310X_BRGCFG_2XMODE_BIT;
		clk = fref * 2;
		div = clk / baud;
		if ((clk % baud) && ((div / 16) < 0x8000))
		{
			/* Mode x4 */
			mode = MAX310X_BRGCFG_4XMODE_BIT;
			clk = fref * 4;
			div = clk / baud;
		}
	}

	RETURN_ON_ERR(max310x_port_write(port, MAX310X_BRGDIVMSB_REG, (div / 16) >> 8));
	RETURN_ON_ERR(max310x_port_write(port, MAX310X_BRGDIVLSB_REG, div / 16));
	RETURN_ON_ERR(max310x_port_write(port, MAX310X_BRGCFG_REG, (div % 16) | mode));
	if(actualBaud != NULL)
		*actualBaud = clk / div; //actual baudrate, this will never be exactly the value requested..
	return Result::Ok;
}

Result MAX14830::portInit(uint8_t port)
{
	uint8_t dummy;

	// Configure MODE2 register 
	RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE2_REG, MAX310X_MODE2_RXEMPTINV_BIT, MAX310X_MODE2_RXEMPTINV_BIT));

	// Clear IRQ status registers
	RETURN_ON_ERR(max310x_port_read(port, MAX310X_IRQSTS_REG, &dummy));
	RETURN_ON_ERR(max310x_port_read(port, MAX310X_LSR_IRQSTS_REG, &dummy));
	RETURN_ON_ERR(max310x_port_read(port, MAX310X_SPCHR_IRQSTS_REG, &dummy));
	RETURN_ON_ERR(max310x_port_read(port, MAX310X_STS_IRQSTS_REG, &dummy));
	RETURN_ON_ERR(max310x_port_read(port, MAX310X_GLOBALIRQ_REG, &dummy));

	// Route GlobalIRQ to IRQPIN
	RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE1_REG, MAX310X_MODE1_IRQSEL_BIT, MAX310X_MODE1_IRQSEL_BIT));

	/* Enable STS, RX, TX, CTS change interrupts */
	RETURN_ON_ERR(max310x_port_write(port, MAX310X_IRQEN_REG, MAX310X_IRQ_RXEMPTY_BIT | MAX310X_IRQ_STS_BIT));
	RETURN_ON_ERR(max310x_port_write(port, MAX310X_LSR_IRQEN_REG, 0));
	RETURN_ON_ERR(max310x_port_write(port, MAX310X_SPCHR_IRQEN_REG, 0));
	
	return Result::Ok;
}

Result MAX14830::GpioConfigure(uint32_t port, uint8_t mask, const GpioConfig *config)
{
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
	uint8_t minimask = mask & 0xF;
	if (minimask == 0)
		return Result::Error;

	//Setup the interrupts
    switch (config->intr)
    {
    case GPIO_CFG_INTR_DISABLE:
		RETURN_ON_ERR(max310x_port_update(port, MAX310X_STS_IRQEN_REG, minimask, 0x00));	//Disable interrupts
        break;
    
	case GPIO_CFG_INTR_ANYEDGE:
		RETURN_ON_ERR(max310x_port_update(port, MAX310X_STS_IRQEN_REG, minimask, 0x0F));	//Enable interrupts
		break;
    default:
        return Result::NotSupported;
    }


	//Setup the pinMode
	uint8_t od = (gpioConfBuffer[port] >> 4) & 0xf;
	uint8_t ou = (gpioConfBuffer[port]) & 0xf;
	switch (config->mode)
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
		return Result::NotSupported;
	}
	
	gpioConfBuffer[port] = (od << 4) | ou;
	RETURN_ON_ERR(max310x_port_write(port, MAX310X_GPIOCFG_REG, gpioConfBuffer[port]));

	//Setup pull up / down
    switch (config->pull)
    {
    case GPIO_CFG_PULL_DISABLE:  //TODO: Implement later!
        break;
    
    default:
        return Result::NotSupported;
    }
    return Result::Ok;
}

Result MAX14830::GpioRead(uint32_t port, uint8_t mask, uint8_t * value)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
	uint8_t minimask = mask & 0xF;
	if (minimask > 0)
	{
		uint8_t reg;
		RETURN_ON_ERR(max310x_port_read(port, MAX310X_GPIODATA_REG, &reg));
		*value &= ~minimask;
		*value |= (reg>>4) & minimask;
	}
	return Result::Ok;
}

Result MAX14830::GpioWrite(uint32_t port, uint8_t mask, uint8_t value)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
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
	return Result::Ok;
}

Result MAX14830::GpioIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback)
{
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
    // Store the callback in a member variable to ensure its lifetime.
    std::shared_ptr<IsrHandle> handle = std::make_shared<IsrHandle>();
    handle->device = this;
	handle->port = port;
    handle->pin = pin;
    handle->callback = callback;
    callbacks.push_back(handle);
    return Result::Ok;
}

Result MAX14830::GpioIsrRemoveCallback(uint32_t port, uint8_t pin)
{
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
    // Search for the callback associated with the GPIO pin and remove it from the list
    for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
        if ((*it)->device == this && (*it)->port == port && (*it)->pin == pin) {
            callbacks.erase(it);
            return Result::Ok; // Callback removed successfully
        }
    }

    // If the callback is not found, return an error
    return Result::Error;
}

Result MAX14830::UartConfigure(uint8_t port, const UartConfig * config)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);

	uint8_t flowCtrlRegVal = 0;

	//TODO: Implement other flow control settings.
	switch (config->flowCtrl)
	{
	case UartConfigFlowControl::UART_CFG_FLOW_CTRL_NONE:
		RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE1_REG, MAX310X_MODE1_TXDIS_BIT, 0));
		RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE1_REG, (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT), (0 | MAX310X_MODE1_IRQSEL_BIT)));
		RETURN_ON_ERR(max310x_port_write(port, MAX310X_HDPIXDELAY_REG, 0));
		break;

	case UartConfigFlowControl::UART_CFG_FLOW_CTRL_RS485:
		RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE1_REG, (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT), (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT)));
		RETURN_ON_ERR(max310x_port_write(port, MAX310X_HDPIXDELAY_REG, 0x11));
		break;
	
	case UartConfigFlowControl::UART_CFG_FLOW_CTRL_RTS_CTS:
		RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE1_REG, MAX310X_MODE1_TXDIS_BIT, MAX310X_MODE1_TXDIS_BIT));
		RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE1_REG, (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT), (0 | MAX310X_MODE1_IRQSEL_BIT)));
		RETURN_ON_ERR(max310x_port_write(port, MAX310X_HDPIXDELAY_REG, 0));
		flowCtrlRegVal |= MAX310X_FLOWCTRL_AUTOCTS_BIT;
		break;
	default:
		ESP_LOGE(TAG, "Flow control setting not implemented %d", (int)config->flowCtrl);
		return Result::NotSupported;
	}

	//TODO: Implement other parity settings.
	switch (config->parity)
	{
	case UartConfigParity::UART_CFG_PARITY_NONE:
		break;
	
	default:
		ESP_LOGE(TAG, "Parity setting not implemented %d", (int)config->parity);
		return Result::NotSupported;
	}

	//TODO: Implement other databits settings.
	switch (config->dataBits)
	{
	case UartConfigDataBits::UART_CFG_DATA_BITS_8:
		break;
	
	default:
		ESP_LOGE(TAG, "Data bits setting not implemented %d", (int)config->dataBits);
		return Result::NotSupported;
	}

	//TODO: Implement other stopbits settings.
	switch (config->stopBits)
	{
	case UartConfigStopBits::UART_CFG_STOP_BITS_1:
		break;
	
	default:
		ESP_LOGE(TAG, "Stop bits setting not implemented %d", (int)config->stopBits);
		return Result::NotSupported;
	}
	
	RETURN_ON_ERR(max310x_set_baud(port, config->baudrate, NULL));
	RETURN_ON_ERR(max310x_port_write(port, MAX310X_LCR_REG, MAX310X_LCR_LENGTH0_BIT | MAX310X_LCR_LENGTH1_BIT));	// 8 bit - no parity - 1 stopbit
	RETURN_ON_ERR(max310x_port_write(port, MAX310X_FLOWCTRL_REG, flowCtrlRegVal));
	
	// Reset FIFOs
	RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE2_REG, MAX310X_MODE2_FIFORST_BIT, MAX310X_MODE2_FIFORST_BIT));
	RETURN_ON_ERR(max310x_port_update(port, MAX310X_MODE2_REG, MAX310X_MODE2_FIFORST_BIT, 0));

	return Result::Ok;
}

Result MAX14830::StreamWrite(uint8_t port, const uint8_t * data, size_t txLen, size_t * written, TickType_t timeout)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
	uint8_t fifolvl;
	RETURN_ON_ERR(max310x_port_read(port, MAX310X_TXFIFOLVL_REG, &fifolvl));	

	if (txLen > SOC_SPI_MAXIMUM_BUFFER_SIZE)
		txLen = SOC_SPI_MAXIMUM_BUFFER_SIZE;
	
	if (txLen > MAX14830_FIFO_MAX - fifolvl)
		txLen = MAX14830_FIFO_MAX - fifolvl;

	//TODO: USE DMA! If we do decide to use DMA, ensure chip select logic is protected!
	if (txLen > 0)
	{
		if(written != NULL)
			*written = txLen;
		return Max14830_WriteBufferPolled(((uint32_t)port << 5), data, txLen);
	}
	if(written != NULL)
		*written = 0;
	return Result::Ok;
}

Result MAX14830::StreamRead(uint8_t port, uint8_t * data, size_t size, size_t* read, TickType_t timeout)
{
	//Wait for RX ISR
	if(!uartRxSemaphores[port].Take(timeout))
		return Result::Error;

	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
	uint8_t rxlen;
	RETURN_ON_ERR(max310x_port_read(port, MAX310X_RXFIFOLVL_REG, &rxlen));		

	if (rxlen > 0)
	{
		if (rxlen > size)
			rxlen = size;
		
		if (rxlen > SOC_SPI_MAXIMUM_BUFFER_SIZE)
			rxlen = SOC_SPI_MAXIMUM_BUFFER_SIZE;
		
		//TODO: USE DMA! If we do decide to use DMA, ensure chip select logic is protected!
		RETURN_ON_ERR(Max14830_ReadBufferPolled(((uint32_t)port << 5), NULL, data, rxlen));
		*read = rxlen;

		// If data is received while we are reading it, the ISR won't be set. (Probably because the buffer never went completely empty?) 
		// So, if data is available after reading, set the semaphore, we do want the caller to process data inbetween so we need to return.
		// Otherwise the caller buffer could be full before we have read everything.
		RETURN_ON_ERR(max310x_port_read(port, MAX310X_RXFIFOLVL_REG, &rxlen));		
		if(rxlen > 0)
		{
			uartRxSemaphores[port].Give();
		}
	}
	
	return Result::Ok;
}


