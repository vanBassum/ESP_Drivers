#include "device.h"
#include "esp_log.h"

DeviceResult SpiDevice::setDeviceConfig(IDeviceConfig &config)
{
	ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiBus", &spiBusKey), DeviceStatus::ConfigError, DeviceResult::ConfigError, TAG, "No property found for 'spiBus'");
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

	const char *customCsPin = nullptr; // Use nullptr for clarity
	if (config.getProperty("customCS", &customCsPin))
	{
		ESP_LOGI(TAG, "customCS = '%s'", customCsPin);

		if (sscanf(customCsPin, "%m[^,],%hhu,%hhu", &csDeviceKey, &csPort, &csPin) == 3)
		{
			devConfig.pre_cb = Select;
			devConfig.post_cb = Deselect;
			ESP_LOGI(TAG, "Custom CS = %s, Port = %hhu, Pin = %hhu", csDeviceKey, csPort, csPin);
		}
	}

	setStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult SpiDevice::loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(spiBus, deviceManager->getDeviceByKey<SpiBus>(spiBusKey), DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "spiBus not available");

	if (csDeviceKey != nullptr)
		GET_DEV_OR_RETURN(csDevice, deviceManager->getDeviceByKey<IGpio>(csDeviceKey), DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "csDevice not available");

	setStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}

DeviceResult SpiDevice::init()
{
	ContextLock lock(mutex);
	spi_host_device_t host;
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiBus->GetHost(&host) == DeviceResult::Ok, DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "spiBus->GetHost error");

	if (spi_bus_add_device(host, &devConfig, &handle) != ESP_OK)
	{
		setStatus(DeviceStatus::Error);
		return DeviceResult::InitFault;
	}

	// TODO: Initialize the driver here! Dont forget to check the result of the functions you call from the dependecies.
	setStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult SpiDevice::Transmit(uint8_t *txData, uint8_t *rxData, size_t size)
{
	ContextLock lock(mutex);
	DEV_RETURN_ON_FALSE(checkDeviceStatus(DeviceStatus::Ready), DeviceResult::WrongStatus, TAG, "Driver not ready, status %d", (int)getDeviceStatus());

	spi_transaction_t transaction{};
	transaction.length = size * 8; // In bits
	transaction.tx_buffer = txData;
	transaction.rx_buffer = rxData;
	transaction.user = this;

	if (spi_device_transmit(handle, &transaction) != ESP_OK)
	{
		setStatus(DeviceStatus::Dependencies);
		return DeviceResult::Dependency;
	}
	return DeviceResult::Ok;
}

void IRAM_ATTR SpiDevice::Select(spi_transaction_t *t)
{
	SpiDevice *device = (SpiDevice *)t->user;
	device->csDevice->portWrite(device->csPort, 1 << device->csPin, 1 << device->csPin); // Opted to not do checks, this function should remain small.
}

void IRAM_ATTR SpiDevice::Deselect(spi_transaction_t *t)
{
	SpiDevice *device = (SpiDevice *)t->user;
	device->csDevice->portWrite(device->csPort, 1 << device->csPin, 0); 				// Opted to not do checks, this function should remain small.
}
