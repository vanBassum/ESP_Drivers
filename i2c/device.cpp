#include "device.h"
#include "bus.h"

esp_err_t I2C::Device::Init(Bus *bus, uint16_t device_address)
{
    this->address = device_address;
    this->bus = bus;
    return ESP_OK;
}

esp_err_t I2C::Device::ReadWrite(const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size)
{
    return i2c_master_write_read_device(bus->host, address, write_buffer, write_size, read_buffer, read_size, portMAX_DELAY);
}

esp_err_t I2C::Device::Write(const uint8_t *write_buffer, size_t write_size)
{
    return i2c_master_write_to_device(bus->host, address, write_buffer, write_size, 1000 / portTICK_PERIOD_MS);
}
