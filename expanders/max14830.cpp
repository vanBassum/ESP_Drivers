#include "max14830.h"
#include "max310x.h"
#include "esp_log.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAX14830";


esp_err_t ESP_Drivers::MAX14830::Init(SPIBus* spiBus, gpio_num_t cs, gpio_num_t irq, transaction_cb_t pre_cb, transaction_cb_t post_cb)
{
	memset(gpioConfBuffer, 0, sizeof(gpioConfBuffer));
	memset(gpioDataBuffer, 0, sizeof(gpioDataBuffer));
	memset(gpioIRQBuffer, 0, sizeof(gpioIRQBuffer));
	esp_err_t result;
	spi_device_interface_config_t spi_devcfg;
	memset(&spi_devcfg, 0, sizeof(spi_devcfg));
	spi_devcfg.clock_speed_hz = 10 * 1000 * 1000;           //Should be able to go higher, for now start low for testing.
	spi_devcfg.mode = 0;
	spi_devcfg.queue_size = 7;
	spi_devcfg.command_bits = 8;
	spi_devcfg.spics_io_num = cs;
	spi_devcfg.pre_cb = pre_cb;
	spi_devcfg.post_cb = post_cb;
	result = spidev.Init(spiBus, &spi_devcfg);	
	if (result != ESP_OK)
		return result;
	
	result = spidev.AcquireBus();
	if (result != ESP_OK)
		return result;
	
	result = Detect();	//is there a max14830 available
	if (result != ESP_OK)
		return result;
	
	max310x_set_ref_clk();
	spidev.ReleaseBus();
	
	if (clockErr)
		return ESP_FAIL;
	
	//This task will do everything required to handle device interrupts.
	//Its optional, don't start the task if interupts aren't required.
	irqPin = irq;
	irqTask.Init("MAX14830", 10, 1048 * 2);
	irqTask.Bind(this, &ESP_Drivers::MAX14830::IrqTaskWork);
	irqTask.Run();
	return result;
}


esp_err_t ESP_Drivers::MAX14830::Detect()
{
	esp_err_t ret = regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_ENBL);
	if (ret == ESP_OK)
	{
		uint8_t value = max310x_port_read(MAX14830_UART_NUM_0, MAX310X_REVID_EXTREG);
		ret = regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_DSBL);
		if (((value & MAX310x_REV_MASK) != MAX14830_REV_ID))
		{
			ESP_LOGE(TAG, "Chip not found revid %d", value);
			return ESP_FAIL;
		}
		//ESP_LOGI(TAG, "Max14830 chip found revid %d", value);
		return ESP_OK;
	}
	return ESP_FAIL;
}


esp_err_t ESP_Drivers::MAX14830::regmap_write(uint8_t cmd, uint8_t value)
{
	return Max14830_WriteBufferPolled(cmd, &value, 1);
}


esp_err_t ESP_Drivers::MAX14830::regmap_read(uint8_t cmd, uint8_t * value)
{
	uint8_t cmdData[1];
	return Max14830_ReadBufferPolled(cmd, cmdData, value, 1);
}


uint8_t ESP_Drivers::MAX14830::max310x_port_read(max14830_uart_port_t port, uint8_t cmd)
{
	uint8_t value = 0;
	cmd = (port << 5) | cmd;
	regmap_read(cmd, &value);
	return value;
}


void ESP_Drivers::MAX14830::max310x_port_write(max14830_uart_port_t port, uint8_t cmd, uint8_t value)
{
	cmd = (port << 5) | cmd;
	regmap_write(cmd, value);
}


void ESP_Drivers::MAX14830::max310x_port_update(max14830_uart_port_t port, uint8_t cmd, uint8_t mask, uint8_t value)
{
	uint8_t val = max310x_port_read(port, cmd);
	val &= ~mask;
	val |= (mask & value);
	max310x_port_write(port, cmd, val);
}


esp_err_t ESP_Drivers::MAX14830::Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t * cmdData, uint8_t count)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       				//Zero out the transaction
	t.length = (count * 8);                   		//amount of bits
	t.tx_buffer = cmdData;               			//The data is the cmd itself
	t.cmd = 0x80 | cmd;								//Add write bit
	return spidev.PollingTransmit(&t);  			//Transmit!
}


esp_err_t ESP_Drivers::MAX14830::Max14830_ReadBufferPolled(uint8_t cmd, uint8_t * cmdData, uint8_t * replyData, uint8_t count)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       				//Zero out the transaction
	t.length = (count * 8);              			//amount of bits
	t.tx_buffer = cmdData;               			//The data is the cmd itself
	t.rx_buffer = replyData;
	t.cmd = cmd;
	return spidev.PollingTransmit(&t);  			//Transmit!
}

uint32_t ESP_Drivers::MAX14830::max310x_set_ref_clk()
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
			vTaskDelay(20 / portTICK_PERIOD_MS);
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


uint8_t ESP_Drivers::MAX14830::max310x_update_best_err(uint64_t f, int64_t *besterr)
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

uint32_t ESP_Drivers::MAX14830::max310x_get_ref_clk()
{
	uint64_t clk = MAX14830_CLK;	//we only need 64bits for calculation..
	uint8_t value = max310x_port_read(MAX14830_UART_NUM_0, MAX310X_PLLCFG_REG);
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

uint32_t ESP_Drivers::MAX14830::max310x_set_baud(max14830_uart_port_t port, uint32_t baud)
{
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

void ESP_Drivers::MAX14830::SetPinsMode(max14830_pins_t mask, max14830_pinmodes_t mode)
{
	spidev.AcquireBus();
	for (int i = 0; i < 4; i++)
	{
		uint8_t minimask = (mask >> (4*i)) & 0xF;
		if (minimask > 0)
		{
			uint8_t od = (gpioConfBuffer[i] >> 4) & 0xf;
			uint8_t ou = (gpioConfBuffer[i]) & 0xf;
			switch (mode)
			{
			case MAX14830_PINMODE_INPUT:
				ou &= ~minimask;
				break;
			case MAX14830_PINMODE_PUSHPULL:
				ou |= minimask;
				od &= ~minimask;
				break;
			case MAX14830_PINMODE_OPENDRAIN:
				ou |= minimask;
				od |= minimask;
				break;
			}
			
			gpioConfBuffer[i] = (od << 4) | ou;
			max310x_port_write((max14830_uart_port_t)i, MAX310X_GPIOCFG_REG, gpioConfBuffer[i]);
		}
	}
	spidev.ReleaseBus();
}

void ESP_Drivers::MAX14830::SetPins(max14830_pins_t mask, max14830_pins_t value)
{
	spidev.AcquireBus();
	for (int i = 0; i < 4; i++)
	{
		uint8_t minimask = (mask >> (4*i)) & 0xF;
		uint8_t minivalue = (value >> (4*i)) & 0xF;
		if (minimask > 0)
		{
			uint8_t ou = (gpioDataBuffer[i]) & 0xf;
			ou &= ~minimask;
			ou |= minivalue & minimask;
			gpioDataBuffer[i] = ou;
			max310x_port_write((max14830_uart_port_t)i, MAX310X_GPIODATA_REG, gpioDataBuffer[i]);
		}
	}
	spidev.ReleaseBus();
}

void ESP_Drivers::MAX14830::SetInterrupts(max14830_pins_t mask, max14830_pins_t value)
{
	spidev.AcquireBus();
	for (int i = 0; i < 4; i++)
	{
		uint8_t minimask = (mask >> (4*i)) & 0xF;
		uint8_t minivalue = (value >> (4*i)) & 0xF;
		if (minimask > 0)
		{
			uint8_t ou = (gpioIRQBuffer[i]) & 0xf;
			ou &= ~minimask;
			ou |= minivalue & minimask;
			gpioIRQBuffer[i] = ou;
			max310x_port_write((max14830_uart_port_t)i, MAX310X_STS_IRQEN_REG, gpioIRQBuffer[i]);
		}
	}
	spidev.ReleaseBus();
}

ESP_Drivers::max14830_pins_t ESP_Drivers::MAX14830::GetPins(max14830_pins_t mask)
{
	spidev.AcquireBus();
	max14830_pins_t result = MAX14830_PIN_NONE;
	for (int i = 0; i < 4; i++)
	{
		uint8_t minimask = (mask >> (4*i)) & 0xF;
		if (minimask > 0)
		{
			uint8_t reg = max310x_port_read((max14830_uart_port_t)i, MAX310X_GPIODATA_REG);
			reg = (reg >> 4) & minimask;
			result |= (max14830_pins_t)(reg << (i * 4));
		}
	}
	spidev.ReleaseBus();
	return result;
}



ESP_Drivers::MAX14830::Uart::Uart(MAX14830* parent, 	max14830_uart_port_t port)
{
	this->parent = parent;
	this->port = port;
}


void ESP_Drivers::MAX14830::Uart::Init(uint32_t baudrate, uint8_t useCTS, uint8_t useRS485)
{
	inputBuffer.Init(64, 1);
	outputBuffer.Init(64, 1);
	
	outputBuffer.OnDataReady.Bind(this, &ESP_Drivers::MAX14830::Uart::OnDataReady);
	
	parent->spidev.AcquireBus();
	uint8_t flowCtrlRegVal = 0;
	if (useCTS)
	{
		//Set transmitter off
		parent->max310x_port_update(port, MAX310X_MODE1_REG, MAX310X_MODE1_TXDIS_BIT, MAX310X_MODE1_TXDIS_BIT);
		flowCtrlRegVal |= MAX310X_FLOWCTRL_AUTOCTS_BIT;
	}
	else
	{
		parent->max310x_port_update(port, MAX310X_MODE1_REG, MAX310X_MODE1_TXDIS_BIT, 0);
	}
	
	parent->max310x_set_baud(port, baudrate);
	parent->max310x_port_write(port, MAX310X_LCR_REG, MAX310X_LCR_LENGTH0_BIT | MAX310X_LCR_LENGTH1_BIT);	// 8 bit - no parity - 1 stopbit
	parent->max310x_port_write(port, MAX310X_FLOWCTRL_REG, flowCtrlRegVal);
	
	if (useRS485)
	{
		parent->max310x_port_update(port, MAX310X_MODE1_REG, (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT), (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT));
		parent->max310x_port_write(port, MAX310X_HDPIXDELAY_REG, 0x11);
	}
	else
	{
		parent->max310x_port_update(port, MAX310X_MODE1_REG, (MAX310X_MODE1_TRNSCVCTRL_BIT | MAX310X_MODE1_IRQSEL_BIT), (0 | MAX310X_MODE1_IRQSEL_BIT));
		parent->max310x_port_write(port, MAX310X_HDPIXDELAY_REG, 0);
	}
	
	
	// Configure MODE2 register & Reset FIFOs
	parent->max310x_port_write(port, MAX310X_MODE2_REG, MAX310X_MODE2_RXEMPTINV_BIT | MAX310X_MODE2_FIFORST_BIT);
	parent->max310x_port_update(port, MAX310X_MODE2_REG, MAX310X_MODE2_FIFORST_BIT, 0);

	// Clear IRQ status registers
	parent->max310x_port_read(port, MAX310X_IRQSTS_REG);
	parent->max310x_port_read(port, MAX310X_LSR_IRQSTS_REG);
	parent->max310x_port_read(port, MAX310X_SPCHR_IRQSTS_REG);
	parent->max310x_port_read(port, MAX310X_STS_IRQSTS_REG);
	parent->max310x_port_read(port, MAX310X_GLOBALIRQ_REG);
	
	/* Enable STS, RX, TX, CTS change interrupts */
	parent->max310x_port_write(port, MAX310X_IRQEN_REG, MAX310X_IRQ_RXEMPTY_BIT | MAX310X_IRQ_STS_BIT);
	parent->max310x_port_write(port, MAX310X_LSR_IRQEN_REG, 0);
	parent->max310x_port_write(port, MAX310X_SPCHR_IRQEN_REG, 0);
	parent->spidev.ReleaseBus();
}


size_t ESP_Drivers::MAX14830::Uart::Write(const void* data, size_t size)
{
	return outputBuffer.Write(data, size);
}

size_t ESP_Drivers::MAX14830::Uart::Read(void* data, size_t size)
{	
	return inputBuffer.Read(data, size);
}

void ESP_Drivers::MAX14830::gpio_isr_handler(void* arg)
{
	MAX14830* parent = (MAX14830 *)arg;
	gpio_set_intr_type(parent->irqPin, GPIO_INTR_DISABLE);	//Stop interrupts, let task handle stuff and re-enable the interrupts.
	parent->irqTask.NotifyFromISR(MAX14830_EVENT_IRQ);	
}

void ESP_Drivers::MAX14830::IrqTaskWork(Task* task, void* args)
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
	//gpio_set_intr_type(irqPin, GPIO_INTR_LOW_LEVEL);	//https://github.com/espressif/esp-idf/issues/4203 'costaud' commented on 28 Oct 2019
	max14830_events_t notifications;
	
	while (true)
	{
		gpio_set_intr_type(irqPin, GPIO_INTR_LOW_LEVEL);	//Re enable interrupt.
		if (task->NotifyWait((uint32_t*)&notifications))
		{
			max14830_pins_t changedPins = MAX14830_PIN_NONE;
			spidev.AcquireBus();
			
			if (HAS_BIT(notifications, MAX14830_EVENT_IRQ))
			{
				//ESP_LOGI(TAG, "IRQ handeling");
				uint8_t uart = 0;				
				regmap_read(MAX310X_GLOBALIRQ_REG, &uart);
				Uart0.HandleIRQ(&changedPins);
				Uart1.HandleIRQ(&changedPins);
				Uart2.HandleIRQ(&changedPins);
				Uart3.HandleIRQ(&changedPins);
			}
			
			
			if (HAS_BIT(notifications, MAX14830_EVENT_PORT0_TX))
				Uart0.HandleOutputBuffer();
			if (HAS_BIT(notifications, MAX14830_EVENT_PORT1_TX))
				Uart1.HandleOutputBuffer();
			if (HAS_BIT(notifications, MAX14830_EVENT_PORT2_TX))
				Uart2.HandleOutputBuffer();
			if (HAS_BIT(notifications, MAX14830_EVENT_PORT3_TX))
				Uart3.HandleOutputBuffer();
			
			spidev.ReleaseBus();
			
			if (changedPins) OnPinsChanged.Invoke(this, changedPins);
		}
	}
}


void ESP_Drivers::MAX14830::Uart::HandleIRQ(max14830_pins_t* changes)
{
	//BUS is already aquired!
	uint8_t isr = parent->max310x_port_read(port, MAX310X_IRQSTS_REG);							
	if (HAS_BIT(isr, MAX310X_IRQ_STS_BIT))
	{
		uint8_t sts = parent->max310x_port_read(port, MAX310X_STS_IRQSTS_REG);		//For GPIO	last 4 bits
		*changes |= (max14830_pins_t)((sts & 0xf) << (port * 4));
	}
				
	//Im missing data, so this bit seems to be unreliable. 
	//Disabling the check for now, doesn't really hurt since we ask the max how many characters are available anyway.
	//if (HAS_BIT(isr, MAX310X_IRQ_RXEMPTY_BIT)) //inverted in the config
	{
		//MAX has uart data for this port, so read it and store it in the buffers.
		size_t rxlen = parent->max310x_port_read(port, MAX310X_RXFIFOLVL_REG);	
		size_t space = inputBuffer.GetAvailableSpace();
		if (rxlen > 0 && space > 0)
		{
			uint8_t buffer[32];
			
			if (rxlen > space)
				rxlen = space;
			
			if (rxlen > sizeof(buffer))
				rxlen = sizeof(buffer);
		
			if (rxlen > SOC_SPI_MAXIMUM_BUFFER_SIZE)
				rxlen = SOC_SPI_MAXIMUM_BUFFER_SIZE;
			
			//TODO: USE DMA! If we do decide to use DMA, ensure chip select logic is protected!
			if (rxlen > 0)
			{
				parent->Max14830_ReadBufferPolled((port << 5), NULL, buffer, rxlen);
				inputBuffer.Write(buffer, rxlen);
			}
		}
	}
}


void ESP_Drivers::MAX14830::Uart::OnDataReady(IStream* buffer)
{
	switch (port)
	{
	case MAX14830_UART_NUM_0:
		parent->irqTask.Notify(MAX14830_EVENT_PORT0_TX);
		break;
	case MAX14830_UART_NUM_1:
		parent->irqTask.Notify(MAX14830_EVENT_PORT1_TX);
		break;
	case MAX14830_UART_NUM_2:
		parent->irqTask.Notify(MAX14830_EVENT_PORT2_TX);
		break;
	case MAX14830_UART_NUM_3:
		parent->irqTask.Notify(MAX14830_EVENT_PORT3_TX);
		break;
		
	}
	
}


void ESP_Drivers::MAX14830::Uart::HandleOutputBuffer()
{
	//BUS is already aquired!
	uint8_t buffer[32];
	size_t txLen = outputBuffer.GetAvailableBytes();
	size_t fifolvl = parent->max310x_port_read(port, MAX310X_TXFIFOLVL_REG);
	
	if (txLen > sizeof(buffer))
		txLen = sizeof(buffer);
	
	if (txLen > SOC_SPI_MAXIMUM_BUFFER_SIZE)
		txLen = SOC_SPI_MAXIMUM_BUFFER_SIZE;
	
	if (txLen > MAX14830_FIFO_MAX - fifolvl)
		txLen = MAX14830_FIFO_MAX - fifolvl;
	
	outputBuffer.Read(buffer, txLen);
	
	//TODO: USE DMA! If we do decide to use DMA, ensure chip select logic is protected!
	if (txLen > 0)
		parent->Max14830_WriteBufferPolled((port << 5), (uint8_t*)buffer, txLen);
	
}
