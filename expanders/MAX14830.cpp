#include "MAX14830.h"
#include "max310x.h"

#define MAX14830_BRGCFG_CLKDIS_BIT		(1 << 6) /* Clock Disable */
#define MAX14830_REV_ID					(0xb0)
#define MAX14830_USE_XTAL
#define MAX14830_CLK					4000000
#define MAX14830_FIFO_MAX				128


DeviceResult MAX14830::setDeviceConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiDevice", &spiDeviceKey),  DeviceStatus::ConfigError, DeviceResult::ConfigError, TAG, "Missing parameter: spiDevice");

    const char *isrPinStr = nullptr;
    DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("isrPin", &isrPinStr),  DeviceStatus::ConfigError, DeviceResult::ConfigError, TAG, "Missing parameter: isrPin");
    DEV_SET_STATUS_AND_RETURN_ON_FALSE(sscanf(isrPinStr, "%m[^,],%hhu,%hhu", &isrDeviceKey, &isrPort, &isrPin) == 3,  DeviceStatus::ConfigError, DeviceResult::ConfigError, TAG, "Error parsing isrPin");
	setStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult MAX14830::loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(spiDevice, deviceManager->getDeviceByKey<SpiDevice>(spiDeviceKey), DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "Missing dependency: spiDevice");
	GET_DEV_OR_RETURN(isrDevice, deviceManager->getDeviceByKey<IGpio>(isrDeviceKey), DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "Missing dependency: isrDevice");
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


    // Init the task
    task.Init("MAX14830", 7, 1024 * 2); //TODO: Should we make these params properies?
    task.SetHandler([&](){Work();});
    task.Run();
	
	// Tell the driver the device is initialized and ready to use.
	setStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}




DeviceResult MAX14830::Detect(bool* result)
{
    uint8_t value;
    DEV_RETURN_ON_ERROR_SILENT(regmap_write(MAX310X_GLOBALCMD_REG, MAX310X_EXTREG_ENBL));
    DEV_RETURN_ON_ERROR_SILENT(max310x_port_read(Ports::NUM_0, MAX310X_REVID_EXTREG, &value));
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
    DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiDevice->Transmit(&t, SPIFlags::POLLED) == DeviceResult::Ok, DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "Error in spi transmit");
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
    DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiDevice->Transmit(&t, SPIFlags::POLLED) == DeviceResult::Ok, DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "Error in spi transmit");
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

DeviceResult MAX14830::max310x_port_write(Ports port, uint8_t cmd, uint8_t value)
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



void MAX14830::Work()
{
    while(1)
    {
        vTaskDelay(1000);
    }
}



