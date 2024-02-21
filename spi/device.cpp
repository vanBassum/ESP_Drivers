#include "device.h"
#include "esp_log.h"

Result SpiDevice::DeviceSetConfig(IDeviceConfig &config)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(config.getProperty("spiBus", &spiBusKey));
	config.getProperty("command_bits", &devConfig.command_bits);
	config.getProperty("address_bits", &devConfig.address_bits);
	config.getProperty("dummy_bits", &devConfig.dummy_bits);
	config.getProperty("mode", &devConfig.mode);
	config.getProperty("duty_cycle_pos", &devConfig.duty_cycle_pos);
	config.getProperty("cs_ena_pretrans", &devConfig.cs_ena_pretrans);
	config.getProperty("cs_ena_posttrans", &devConfig.cs_ena_posttrans);
	config.getProperty("clock_speed_hz", &devConfig.clock_speed_hz);
	config.getProperty("input_delay_ns", &devConfig.input_delay_ns);
	config.getProperty("spics_io_num", &devConfig.spics_io_num);
	config.getProperty("flags", &devConfig.flags);
	config.getProperty("queue_size", &devConfig.queue_size);

	const char* key = nullptr;
	config.getProperty("key", &key);

	const char *customCsPin = nullptr; // Use nullptr for clarity

	if (config.getProperty("customCS", &customCsPin) == Result::Ok)
	{
		if (sscanf(customCsPin, "%m[^,],%hhu,%hhu", &csDeviceKey, &csPort, &csPin) == 3)
		{
			devConfig.pre_cb = Select;
			devConfig.post_cb = Deselect;
			//ESP_LOGI(TAG, "'%s' Using CS pin, Device ='%s' Port = %hhu, Pin = %hhu", key, csDeviceKey, csPort, csPin);
		}
	}

	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result SpiDevice::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(deviceManager->getDeviceByKey<SpiBus>(spiBusKey, spiBus));

	if (csDeviceKey != nullptr)
		RETURN_ON_ERR(deviceManager->getDeviceByKey<IGpio>(csDeviceKey, csDevice));

	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result SpiDevice::DeviceInit()
{
	ContextLock lock(mutex);
	spi_host_device_t host = SPI_HOST_MAX;
	RETURN_ON_ERR(spiBus->GetHost(&host));

	if (spi_bus_add_device(host, &devConfig, &handle) != ESP_OK)
	{
		DeviceSetStatus(DeviceStatus::EndOfLife);
		return Result::Error;
	}

	// TODO: Initialize the driver here! Dont forget to check the result of the functions you call from the dependecies.
	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
}

Result SpiDevice::SpiTransmit(const uint8_t *txData, uint8_t *rxData, size_t size, SPIFlags flags)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));
	spi_transaction_t transaction = {};
	transaction.length = size * 8; // In bits
	transaction.tx_buffer = txData;
	transaction.rx_buffer = rxData;
	return Transmit(&transaction, flags);
}

Result SpiDevice::Transmit(spi_transaction_t *transaction, SPIFlags flags)
{
	transaction->user = this;
	esp_err_t err = ESP_OK;

	if(flags & SPIFlags::POLLED)
		err = spi_device_polling_transmit(handle, transaction);
	else
		err = spi_device_transmit(handle, transaction);

	if (err != ESP_OK)
	{
		DeviceSetStatus(DeviceStatus::Dependencies);
		const char* err_str = esp_err_to_name(err);
    	ESP_LOGE(TAG, "SPI transaction failed: %s", err_str);
		return Result::Error;
	}
	
	return Result::Ok;
}

void IRAM_ATTR SpiDevice::Select(spi_transaction_t *t)
{
	SpiDevice *device = (SpiDevice *)t->user;
	assert(device);
	device->csDevice->GpioWrite(device->csPort, 1 << device->csPin, 1 << device->csPin); // Opted to not do checks, this function should remain small.
}

void IRAM_ATTR SpiDevice::Deselect(spi_transaction_t *t)
{
	SpiDevice *device = (SpiDevice *)t->user;
	assert(device);
	device->csDevice->GpioWrite(device->csPort, 1 << device->csPin, 0); 				// Opted to not do checks, this function should remain small.
}
