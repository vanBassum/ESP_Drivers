#include "max14830.h"
#include "max310x.h"
#include "esp_log.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


MAX14830::MAX14830(std::shared_ptr<SPIDevice> spi, std::function<void(MAX14830& dev)> configurator)
	: spi(spi)
{
	configurator((*this));

	ContextLock lock(mutex);
	ESP_LOGI(TAG, "Initializing");
	memset(gpioConfBuffer, 0, sizeof(gpioConfBuffer));
	memset(gpioDataBuffer, 0, sizeof(gpioDataBuffer));
	memset(gpioIRQBuffer, 0, sizeof(gpioIRQBuffer));

	spi->AcquireBus();
	assert(Detect());
	max310x_set_ref_clk();
	spi->ReleaseBus();
	
	//This task will do everything required to handle device interrupts.
	//Its optional, don't start the task if interupts aren't required.
	
	irqTask.Init("MAX14830", 10, 1048 * 4);
	irqTask.SetHandler([&](Task* task, void* args){this->IrqTaskWork();});
	irqTask.Run();
}



bool MAX14830::Detect()
{
	ContextLock lock(mutex);
	regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_ENBL);
	uint8_t value = max310x_port_read(Ports::NUM_0, MAX310X_REVID_EXTREG);
	regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_DSBL);
	if (((value & MAX310x_REV_MASK) != MAX14830_REV_ID))
	{
		ESP_LOGE(TAG, "Chip not found revid %d", value);
		return false;
	}
	return true;
}


void MAX14830::regmap_write(uint8_t cmd, uint8_t value)
{
	ContextLock lock(mutex);
	Max14830_WriteBufferPolled(cmd, &value, 1);
}


void MAX14830::regmap_read(uint8_t cmd, uint8_t * value)
{
	ContextLock lock(mutex);
	uint8_t cmdData[1];
	Max14830_ReadBufferPolled(cmd, cmdData, value, 1);
}


uint8_t MAX14830::max310x_port_read(Ports port, uint8_t cmd)
{
	ContextLock lock(mutex);
	uint8_t value = 0;
	cmd = ((uint32_t)port << 5) | cmd;
	regmap_read(cmd, &value);
	return value;
}


void MAX14830::max310x_port_write(Ports port, uint8_t cmd, uint8_t value)
{
	ContextLock lock(mutex);
	cmd = ((uint32_t)port << 5) | cmd;
	regmap_write(cmd, value);
}


void MAX14830::max310x_port_update(Ports port, uint8_t cmd, uint8_t mask, uint8_t value)
{
	ContextLock lock(mutex);
	uint8_t val = max310x_port_read(port, cmd);
	val &= ~mask;
	val |= (mask & value);
	max310x_port_write(port, cmd, val);
}


void MAX14830::Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t * cmdData, uint8_t count)
{
	ContextLock lock(mutex);
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       				//Zero out the transaction
	t.length = (count * 8);                   		//amount of bits
	t.tx_buffer = cmdData;               			//The data is the cmd itself
	t.cmd = 0x80 | cmd;								//Add write bit
	spi->PollingTransmit(&t);  			//Transmit!
}


void MAX14830::Max14830_ReadBufferPolled(uint8_t cmd, uint8_t * cmdData, uint8_t * replyData, uint8_t count)
{
	ContextLock lock(mutex);
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       				//Zero out the transaction
	t.length = (count * 8);              			//amount of bits
	t.tx_buffer = cmdData;               			//The data is the cmd itself
	t.rx_buffer = replyData;
	t.cmd = cmd;
	spi->PollingTransmit(&t);  			//Transmit!
}

uint32_t MAX14830::max310x_set_ref_clk()
{
	ContextLock lock(mutex);
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
		clockErr = true;
	}
#endif
	return (uint32_t)bestfreq;
}


uint8_t MAX14830::max310x_update_best_err(uint64_t f, int64_t *besterr)
{
	ContextLock lock(mutex);
	/* Use baudrate 115200 for calculate error */
	int64_t err = f % (115200 * 16);
	if ((*besterr < 0) || (*besterr > err))
	{
		*besterr = err;
		return 0;
	}
	return 1;
}

uint32_t MAX14830::max310x_get_ref_clk()
{
	ContextLock lock(mutex);
	uint64_t clk = MAX14830_CLK;	//we only need 64bits for calculation..
	uint8_t value = max310x_port_read(Ports::NUM_0, MAX310X_PLLCFG_REG);
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
	clk = (clk * mul) / clkDiv;	//math is turned to prevent math errors (trunc).
	return clk;
}

uint32_t MAX14830::max310x_set_baud(Ports port, uint32_t baud)
{
	ContextLock lock(mutex);
	uint8_t mode = 0;
	uint32_t fref = max310x_get_ref_clk();
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

	max310x_port_write(port, MAX310X_BRGDIVMSB_REG, (div / 16) >> 8);
	max310x_port_write(port, MAX310X_BRGDIVLSB_REG, div / 16);
	max310x_port_write(port, MAX310X_BRGCFG_REG, (div % 16) | mode);
	return clk / div; //actual baudrate, this will never be exactly the value requested..
}

void MAX14830::SetPinsMode(Pins mask, PinModes mode)
{
	ContextLock lock(mutex);
	spi->AcquireBus();
	for (int i = 0; i < 4; i++)
	{
		uint8_t minimask = ((uint32_t)mask >> (4*i)) & 0xF;
		if (minimask > 0)
		{
			uint8_t od = (gpioConfBuffer[i] >> 4) & 0xf;
			uint8_t ou = (gpioConfBuffer[i]) & 0xf;
			switch (mode)
			{
			case PinModes::INPUT:
				ou &= ~minimask;
				break;
			case PinModes::PUSHPULL:
				ou |= minimask;
				od &= ~minimask;
				break;
			case PinModes::OPENDRAIN:
				ou |= minimask;
				od |= minimask;
				break;
			}
			
			gpioConfBuffer[i] = (od << 4) | ou;
			max310x_port_write((Ports)i, MAX310X_GPIOCFG_REG, gpioConfBuffer[i]);
		}
	}
	spi->ReleaseBus();
}

void MAX14830::SetPins(Pins mask, Pins value)
{
	ContextLock lock(mutex);
	spi->AcquireBus();
	for (int i = 0; i < 4; i++)
	{
		uint8_t minimask = ((uint32_t)mask >> (4*i)) & 0xF;
		uint8_t minivalue = ((uint32_t)value >> (4*i)) & 0xF;
		if (minimask > 0)
		{
			uint8_t ou = (gpioDataBuffer[i]) & 0xf;
			ou &= ~minimask;
			ou |= minivalue & minimask;
			gpioDataBuffer[i] = ou;
			max310x_port_write((Ports)i, MAX310X_GPIODATA_REG, gpioDataBuffer[i]);
		}
	}
	spi->ReleaseBus();
}

void MAX14830::SetInterrupts(Pins mask, Pins value)
{
	ContextLock lock(mutex);
	spi->AcquireBus();
	for (int i = 0; i < 4; i++)
	{
		uint8_t minimask = ((uint32_t)mask >> (4*i)) & 0xF;
		uint8_t minivalue = ((uint32_t)value >> (4*i)) & 0xF;
		if (minimask > 0)
		{
			uint8_t ou = (gpioIRQBuffer[i]) & 0xf;
			ou &= ~minimask;
			ou |= minivalue & minimask;
			gpioIRQBuffer[i] = ou;
			max310x_port_write((Ports)i, MAX310X_STS_IRQEN_REG, gpioIRQBuffer[i]);
		}
	}
	spi->ReleaseBus();
}

MAX14830::Pins MAX14830::GetPins(Pins mask)
{
	ContextLock lock(mutex);
	spi->AcquireBus();
	Pins result = Pins::NONE;
	for (int i = 0; i < 4; i++)
	{
		uint8_t minimask = ((uint32_t)mask >> (4*i)) & 0xF;
		if (minimask > 0)
		{
			uint8_t reg = max310x_port_read((Ports)i, MAX310X_GPIODATA_REG);
			reg = (reg >> 4) & minimask;
			result |= (Pins)(reg << (i * 4));
		}
	}
	spi->ReleaseBus();
	return result;
}





void MAX14830::gpio_isr_handler(void* arg)
{
	MAX14830* parent = (MAX14830 *)arg;
	gpio_set_intr_type(parent->irqPin, GPIO_INTR_DISABLE);	//Stop interrupts, let task handle stuff and re-enable the interrupts.
	parent->irqTask.NotifyFromISR((uint32_t)Events::IRQ);	
}

//https://github.com/espressif/esp-idf/issues/4203 'costaud' commented on 28 Oct 2019
void MAX14830::IrqTaskWork()
{
	//Configure GPIO interrupts.
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = (1ULL << irqPin);
	io_conf.mode = GPIO_MODE_INPUT;			
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	
	gpio_config(&io_conf);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(irqPin, gpio_isr_handler, this);
	gpio_set_intr_type(irqPin, GPIO_INTR_LOW_LEVEL);
	//gpio_set_intr_type(irqPin, GPIO_INTR_NEGEDGE);				
	Events notifications;
	
	while (true)
	{
		if (irqTask.NotifyWait((uint32_t*)&notifications))
		{
			ContextLock lock(mutex);
			Pins changedPins = Pins::NONE;
			spi->AcquireBus();
			
			if (HAS_BIT(notifications, Events::IRQ))
			{
				uint8_t uart = 0;				
				regmap_read(MAX310X_GLOBALIRQ_REG, &uart);					
				//These bits are inverted! See datasheet
				if ((uart & 0x01) == 0) HandleIRQ(Ports::NUM_0, &changedPins);
				if ((uart & 0x02) == 0) HandleIRQ(Ports::NUM_1, &changedPins);
				if ((uart & 0x04) == 0) HandleIRQ(Ports::NUM_2, &changedPins);
				if ((uart & 0x08) == 0) HandleIRQ(Ports::NUM_3, &changedPins);
				gpio_set_intr_type(irqPin, GPIO_INTR_LOW_LEVEL);	
			}
			
			spi->ReleaseBus();
			
			if ((uint32_t)changedPins) 
				OnPinsChanged.Invoke(this, changedPins);
		}
	}
}

void MAX14830::HandleIRQ(Ports port, Pins* changes)
{
	ContextLock lock(mutex);
	//BUS is already aquired!
	uint8_t isr = max310x_port_read(port, MAX310X_IRQSTS_REG);							
	if (HAS_BIT(isr, MAX310X_IRQ_STS_BIT))
	{
		uint8_t sts = max310x_port_read(port, MAX310X_STS_IRQSTS_REG);		//For GPIO	last 4 bits
		*changes |= (Pins)((sts & 0xf) << ((uint32_t)port * 4));
	}

	if (HAS_BIT(isr, MAX310X_IRQ_RXEMPTY_BIT)) //inverted in the config
	{
		dataAvailable[(int)port].Give();	//Notifiy the read that there is data in the MAX buffers
	}
}




MAX14830::Uart::Uart(std::shared_ptr<MAX14830> max, std::function<void(Uart& dev)> configurator)
	: max(max)
{
	configurator((*this));
	Init();	//This is a quick fix to enable the GPIO interrupts for all ports
}

void MAX14830::PortInit(Ports port, uint32_t baudrate, bool useCTS, bool useRS485)
{
	ContextLock lock(mutex);
	ESP_LOGI(TAG, "init");
	spi->AcquireBus();
	uint8_t flowCtrlRegVal = 0;
	if (useCTS)
	{
		//Set transmitter off
		max310x_port_update(port, MAX310X_MODE1_REG, MAX310X_MODE1_TXDIS_BIT, MAX310X_MODE1_TXDIS_BIT);
		flowCtrlRegVal |= MAX310X_FLOWCTRL_AUTOCTS_BIT;
	}
	else
	{
		max310x_port_update(port, MAX310X_MODE1_REG, MAX310X_MODE1_TXDIS_BIT, 0);
	}
	
	max310x_set_baud(port, baudrate);
	max310x_port_write(port, MAX310X_LCR_REG, MAX310X_LCR_LENGTH0_BIT | MAX310X_LCR_LENGTH1_BIT);	// 8 bit - no parity - 1 stopbit
	max310x_port_write(port, MAX310X_FLOWCTRL_REG, flowCtrlRegVal);
	
	if (useRS485)
	{
		max310x_port_update(port, MAX310X_MODE1_REG, (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT), (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT));
		max310x_port_write(port, MAX310X_HDPIXDELAY_REG, 0x11);
	}
	else
	{
		max310x_port_update(port, MAX310X_MODE1_REG, (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT), (0 | MAX310X_MODE1_IRQSEL_BIT));
		max310x_port_write(port, MAX310X_HDPIXDELAY_REG, 0);
	}
	
	
	// Configure MODE2 register & Reset FIFOs
	max310x_port_write(port, MAX310X_MODE2_REG, MAX310X_MODE2_RXEMPTINV_BIT | MAX310X_MODE2_FIFORST_BIT);
	max310x_port_update(port, MAX310X_MODE2_REG, MAX310X_MODE2_FIFORST_BIT, 0);

	// Clear IRQ status registers
	max310x_port_read(port, MAX310X_IRQSTS_REG);
	max310x_port_read(port, MAX310X_LSR_IRQSTS_REG);
	max310x_port_read(port, MAX310X_SPCHR_IRQSTS_REG);
	max310x_port_read(port, MAX310X_STS_IRQSTS_REG);
	max310x_port_read(port, MAX310X_GLOBALIRQ_REG);
	
	/* Enable STS, RX, TX, CTS change interrupts */
	//max310x_port_write(port, MAX310X_IRQEN_REG, MAX310X_IRQ_RXEMPTY_BIT | MAX310X_IRQ_TXEMPTY_BIT | MAX310X_IRQ_STS_BIT);
	max310x_port_write(port, MAX310X_IRQEN_REG, MAX310X_IRQ_RXEMPTY_BIT | MAX310X_IRQ_STS_BIT);
	max310x_port_write(port, MAX310X_LSR_IRQEN_REG, 0);
	max310x_port_write(port, MAX310X_SPCHR_IRQEN_REG, 0);
	spi->ReleaseBus();
}


size_t MAX14830::PortRead(Ports port, uint8_t* buffer, size_t size)
{
	size_t rxlen = 0;
	if (dataAvailable[(int)port].Take())	//Wait blocking for data
	{
		ContextLock lock(mutex);
		spi->AcquireBus();
		rxlen = max310x_port_read(port, MAX310X_RXFIFOLVL_REG);		

		if (rxlen > 0)
		{
			if (rxlen > size)
				rxlen = size;
			
			if (rxlen > SOC_SPI_MAXIMUM_BUFFER_SIZE)
				rxlen = SOC_SPI_MAXIMUM_BUFFER_SIZE;
			
			//TODO: USE DMA! If we do decide to use DMA, ensure chip select logic is protected!
			Max14830_ReadBufferPolled(((uint32_t)port << 5), NULL, buffer, rxlen);

			// If data is received while we are reading it, the ISR won't be set. 
			// So, if data is available after reading, set the semaphore, we do want the caller to process data inbetween so we need to return.
			// Otherwise the caller buffer could be full before we have read everything.
			size_t more = max310x_port_read(port, MAX310X_RXFIFOLVL_REG);
			if(more > 0)
			{
				dataAvailable[(int)port].Give();
			}
		}
		spi->ReleaseBus();
	}
	return rxlen;
}


size_t MAX14830::PortWrite(Ports port, const uint8_t* buffer, size_t size)
{
	ContextLock lock(mutex);
	spi->AcquireBus();
		
	size_t txLen = size;
	size_t fifolvl = max310x_port_read(port, MAX310X_TXFIFOLVL_REG);		

	if (txLen > SOC_SPI_MAXIMUM_BUFFER_SIZE)
		txLen = SOC_SPI_MAXIMUM_BUFFER_SIZE;
	
	if (txLen > MAX14830_FIFO_MAX - fifolvl)
		txLen = MAX14830_FIFO_MAX - fifolvl;

	//TODO: USE DMA! If we do decide to use DMA, ensure chip select logic is protected!
	if (txLen > 0)
		Max14830_WriteBufferPolled(((uint32_t)port << 5), (uint8_t*)buffer, txLen);

	spi->ReleaseBus();	
	
	return txLen;
}



