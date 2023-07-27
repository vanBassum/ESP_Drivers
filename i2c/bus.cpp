#include "bus.h"
#include "esp_log.h"

I2CBus::I2CBus(i2c_port_t host, const i2c_config_t& config)
{
	ESP_LOGI(TAG, "Initializing");
	this->host = host;
	ESP_ERROR_CHECK(i2c_param_config(host, &config));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, config.mode, 0, 0, 0));
}


I2CBus::~I2CBus()
{
	devices.clear(); //By calling devices.clear() in the destructor, it will remove all the SPIDevice objects from the vector, triggering their destructors and ensuring proper cleanup.
}


I2CDevice& I2CBus::CreateDevice(const uint16_t device_address)
{
	devices.emplace_back(new I2CDevice(*this, device_address));
	return *devices.back();
}
