#include "device.h"
#include "esp_log.h"

void print_spi_device_interface_config(const spi_device_interface_config_t *config) {
    printf("command_bits: %u\n", config->command_bits);
    printf("address_bits: %u\n", config->address_bits);
    printf("dummy_bits: %u\n", config->dummy_bits);
    printf("mode: %u\n", config->mode);
    printf("duty_cycle_pos: %u\n", config->duty_cycle_pos);
    printf("cs_ena_pretrans: %u\n", config->cs_ena_pretrans);
    printf("cs_ena_posttrans: %u\n", config->cs_ena_posttrans);
    printf("clock_speed_hz: %d\n", config->clock_speed_hz);
    printf("input_delay_ns: %d\n", config->input_delay_ns);
    printf("spics_io_num: %d\n", config->spics_io_num);
    printf("flags: %lu\n", config->flags);
    printf("queue_size: %d\n", config->queue_size);
    printf("pre_cb: %p\n", (void *)config->pre_cb);
    printf("post_cb: %p\n", (void *)config->post_cb);
}

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


	if(config.getProperty("customCS", &customCsPin))
	{
		devConfig.pre_cb = Select;
		devConfig.post_cb = Deselect;
		ESP_LOGI(TAG, "Custom CS = %d", customCsPin);
	}
	print_spi_device_interface_config(&devConfig);
	//config.getProperty("pre_cb", &devConfig.pre_cb);
	//config.getProperty("post_cb", &devConfig.post_cb);
	
	setStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult SpiDevice::loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(spiBus, deviceManager->getDeviceByKey<SpiBus>(spiBusKey), DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "Dependencies not ready %d", (int)getDeviceStatus());
	setStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}

DeviceResult SpiDevice::init()
{
	ContextLock lock(mutex);
	spi_host_device_t host;
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiBus->GetHost(&host) == DeviceResult::Ok, DeviceStatus::Dependencies, DeviceResult::Dependency, TAG, "spiBus->GetHost error");

	if(spi_bus_add_device(host, &devConfig, &handle) != ESP_OK)
	{
		setStatus(DeviceStatus::Error);
		return DeviceResult::InitFault;
	}

	//TODO: Initialize the driver here! Dont forget to check the result of the functions you call from the dependecies.
	setStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult SpiDevice::Transmit(uint8_t *txData, uint8_t *rxData, size_t size)
{
	ContextLock lock(mutex);
	DEV_RETURN_ON_FALSE(checkDeviceStatus(DeviceStatus::Ready), DeviceResult::WrongStatus, TAG, "Driver not ready, status %d", (int)getDeviceStatus());

    spi_transaction_t transaction {};
    transaction.length = size * 8; // In bits
    transaction.tx_buffer = txData;
    transaction.rx_buffer = rxData;
	transaction.user = this;

	if(spi_device_polling_transmit(handle, &transaction) != ESP_OK)
	{
		setStatus(DeviceStatus::Dependencies);
		return DeviceResult::Dependency;
	}
	return DeviceResult::Ok;
}

void IRAM_ATTR Select(spi_transaction_t *t)
{
	SpiDevice* device = (SpiDevice*)t->user;  
    gpio_set_level(SPI_CS_PIN_A0, device->customCsPin & 0x01);
	gpio_set_level(SPI_CS_PIN_A1, device->customCsPin & 0x02);
	gpio_set_level(SPI_CS_PIN_A2, device->customCsPin & 0x04);
}

void IRAM_ATTR Deselect(spi_transaction_t *t)
{
    gpio_set_level(SPI_CS_PIN_A0, 0);
	gpio_set_level(SPI_CS_PIN_A1, 0);
	gpio_set_level(SPI_CS_PIN_A2, 0);
}
