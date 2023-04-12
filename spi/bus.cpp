#include "bus.h"
#include "string.h"

esp_err_t SPI::Bus::Init(spi_host_device_t host, gpio_num_t clk, gpio_num_t mosi, gpio_num_t miso, gpio_num_t qwp, gpio_num_t qhd, spi_dma_chan_t dma)
{
	spi_bus_config_t spiConfig;
	memset(&spiConfig, 0, sizeof(spi_bus_config_t));
	spiConfig.sclk_io_num = clk;
	spiConfig.mosi_io_num = mosi;
	spiConfig.miso_io_num = miso;
	spiConfig.quadwp_io_num = qwp;
	spiConfig.quadhd_io_num = qhd;
	spiConfig.max_transfer_sz = 1024;
	_host = host;
	return spi_bus_initialize(host, &spiConfig, dma);
}

