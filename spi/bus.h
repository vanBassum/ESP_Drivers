#pragma once
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "device.h"

namespace ESP_Com
{
	class SPIDevice;
	
	class SPIBus
	{
	protected:
		spi_host_device_t _host;
		friend SPIDevice;
	public:
		esp_err_t Init(spi_host_device_t host, gpio_num_t clk, gpio_num_t mosi, gpio_num_t miso, gpio_num_t qwp = GPIO_NUM_NC, gpio_num_t qhd = GPIO_NUM_NC, spi_dma_chan_t dma = 0);
	};
}




