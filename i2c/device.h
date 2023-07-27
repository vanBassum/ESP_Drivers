#pragma once
#include <esp_system.h>

class I2CBus;
class I2CDevice
{
	I2CBus& bus;
	uint16_t address;
public:
	I2CDevice(I2CBus& bus, const uint16_t device_address);
	void Write(const uint8_t *txData, size_t length);
	void ReadWrite(const uint8_t *txData, size_t txSize, uint8_t *rxData, size_t rxSize);
};
