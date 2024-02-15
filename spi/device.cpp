#include "device.h"
#include "esp_log.h"



IDevice::ErrCode SpiDevice::setConfig(IDeviceConfig &config)
{
	ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiBus", &spiBusKey), Status::ConfigError, ErrCode::ConfigError, TAG, "No property found for 'spiBus'");
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
		
	}

	//config.getProperty("pre_cb", &devConfig.pre_cb);
	//config.getProperty("post_cb", &devConfig.post_cb);
	
	setStatus(Status::Dependencies);
	return ErrCode::Ok;
}

IDevice::ErrCode SpiDevice::loadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(spiBus, deviceManager->getDeviceByKey<SpiBus>(spiBusKey), Status::Dependencies, ErrCode::Dependency, TAG, "Dependencies not ready %d", (int)getStatus());
	setStatus(Status::Initializing);
	return ErrCode::Ok;
}

IDevice::ErrCode SpiDevice::init()
{
	ContextLock lock(mutex);
	spi_host_device_t host;
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiBus->GetHost(&host) == ErrCode::Ok, Status::Dependencies, ErrCode::Dependency, TAG, "spiBus->GetHost error");

	if(spi_bus_add_device(host, &devConfig, &handle) != ESP_OK)
	{
		setStatus(Status::Error);
		return ErrCode::InitFault;
	}

	//TODO: Initialize the driver here! Dont forget to check the result of the functions you call from the dependecies.
	setStatus(Status::Ready);
	return ErrCode::Ok;
}

IDevice::ErrCode SpiDevice::Write(uint8_t *data, size_t size)
{
	ContextLock lock(mutex);
	//Ensure the device is in the ready state
	DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());

	//TODO: Implement writing of data. Dont forget to check the result of the functions you call from the dependecies.
	return ErrCode::Ok;
}

IDevice::ErrCode SpiDevice::Read(uint8_t *data, size_t size)
{
	ContextLock lock(mutex);
	//Ensure the device is in the ready state
	DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());

	//TODO: Implement reading of data. Dont forget to check the result of the functions you call from the dependecies.
	return ErrCode::Ok;
}

IDevice::ErrCode SpiDevice::Transmit(uint8_t *txData, uint8_t *rxData, size_t size)
{
	ContextLock lock(mutex);
	//Ensure the device is in the ready state
	DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());

	//TODO: Implement reading of data. Dont forget to check the result of the functions you call from the dependecies.
	return ErrCode::Ok;
}

void IRAM_ATTR Select(spi_transaction_t *t)
{
    gpio_set_level(SPI_CS_PIN_A0, 0);
	gpio_set_level(SPI_CS_PIN_A1, 0);
	gpio_set_level(SPI_CS_PIN_A2, 0);
}

void IRAM_ATTR Deselect(spi_transaction_t *t)
{
    SpiDevice* device = (SpiDevice*)t->user;  
    gpio_set_level(SPI_CS_PIN_A0, device->customCsPin & 0x01);
	gpio_set_level(SPI_CS_PIN_A1, device->customCsPin & 0x02);
	gpio_set_level(SPI_CS_PIN_A2, device->customCsPin & 0x04);
}
