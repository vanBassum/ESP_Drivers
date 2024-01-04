#include "device.h"
#include "esp_log.h"

SPIDevice::SPIDevice(std::shared_ptr<SPIBus> spiBus) : spiBus(std::move(spiBus)) {
    assert(!spiBus && "SPIBus is nullptr");
}

void SPIDevice::setConfig(const Config& newConfig) {
    assert(!initialized && "Can't change configuration after initialization");
    config = newConfig;
}

void SPIDevice::init() {
    assert(!initialized && "SPIDevice already initialized");
    ESP_LOGI(TAG, "Initializing");
	ESP_ERROR_CHECK(spi_bus_add_device(spiBus->config.host, &config.devConfig, &handle));
    initialized = true;
}

void SPIDevice::transfer(uint8_t* txData, uint8_t* rxData, size_t length) {
	assert(initialized && "SPIDevice not initialized");
    ContextLock lock(mutex);
    spi_transaction_t transaction {};
    transaction.length = length * 8; // In bits
    transaction.tx_buffer = txData;
    transaction.rx_buffer = rxData;
    ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
}

void SPIDevice::PollingTransmit(spi_transaction_t* transaction) {
	assert(initialized && "SPIDevice not initialized");
    ContextLock lock(mutex);
    esp_err_t res = spi_device_polling_transmit(handle, transaction);
    ESP_ERROR_CHECK(res); // Transmit!
}

void SPIDevice::Transmit(spi_transaction_t* transaction) {
	assert(initialized && "SPIDevice not initialized");
    ContextLock lock(mutex);
    ESP_ERROR_CHECK(spi_device_transmit(handle, transaction));
}

void SPIDevice::ReleaseBus() {
	assert(initialized && "SPIDevice not initialized");
    ContextLock lock(mutex);
    spi_device_release_bus(handle);
}

void SPIDevice::AcquireBus() {
	assert(initialized && "SPIDevice not initialized");
    ContextLock lock(mutex);
    ESP_ERROR_CHECK(spi_device_acquire_bus(handle, portMAX_DELAY));
}

bool SPIDevice::isInitialized() const {
    return initialized;
}