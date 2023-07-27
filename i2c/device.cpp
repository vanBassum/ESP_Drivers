#include "device.h"
#include "bus.h"


I2CDevice::I2CDevice(I2CBus& bus, const uint16_t device_address)
	: bus(bus)
	, address(device_address)
{
	
}


void I2CDevice::Write(const uint8_t *txData, size_t length)
{
	ESP_ERROR_CHECK(i2c_master_write_to_device(bus.host, address, txData, length, portMAX_DELAY));
}


void I2CDevice::ReadWrite(const uint8_t *txData, size_t txSize, uint8_t *rxData, size_t rxSize)
{
	ESP_ERROR_CHECK(i2c_master_write_read_device(bus.host, address, txData, txSize, rxData, rxSize, portMAX_DELAY));
}

