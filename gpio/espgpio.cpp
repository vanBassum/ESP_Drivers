#include "espgpio.h"
#include "driver/gpio.h"



DeviceResult EspGpio::setDeviceConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	setStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult EspGpio::loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
    setStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}


DeviceResult EspGpio::init()
{
	ContextLock lock(mutex);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_SHARED);
	setStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult EspGpio::portRead(uint32_t port, uint8_t mask, uint8_t* value)      
{
    ContextLock lock(mutex);
    *value = 0x00;
    if(mask & 0x01) *value += gpio_get_level((gpio_num_t)(port + 0x01)) * 0x01;
    if(mask & 0x02) *value += gpio_get_level((gpio_num_t)(port + 0x02)) * 0x02;
    if(mask & 0x04) *value += gpio_get_level((gpio_num_t)(port + 0x04)) * 0x04;
    if(mask & 0x08) *value += gpio_get_level((gpio_num_t)(port + 0x08)) * 0x08;
    if(mask & 0x10) *value += gpio_get_level((gpio_num_t)(port + 0x10)) * 0x10;
    if(mask & 0x20) *value += gpio_get_level((gpio_num_t)(port + 0x20)) * 0x20;
    if(mask & 0x40) *value += gpio_get_level((gpio_num_t)(port + 0x40)) * 0x40;
    if(mask & 0x80) *value += gpio_get_level((gpio_num_t)(port + 0x80)) * 0x80;
    return DeviceResult::Ok;
}

DeviceResult EspGpio::portWrite(uint32_t port, uint8_t mask, uint8_t value)      
{
    ContextLock lock(mutex);
    if(mask & 0x01) gpio_set_level((gpio_num_t)(port + 0x01), value & 0x01);
    if(mask & 0x02) gpio_set_level((gpio_num_t)(port + 0x02), value & 0x02);
    if(mask & 0x04) gpio_set_level((gpio_num_t)(port + 0x04), value & 0x04);
    if(mask & 0x08) gpio_set_level((gpio_num_t)(port + 0x08), value & 0x08);
    if(mask & 0x10) gpio_set_level((gpio_num_t)(port + 0x10), value & 0x10);
    if(mask & 0x20) gpio_set_level((gpio_num_t)(port + 0x20), value & 0x20);
    if(mask & 0x40) gpio_set_level((gpio_num_t)(port + 0x40), value & 0x40);
    if(mask & 0x80) gpio_set_level((gpio_num_t)(port + 0x80), value & 0x80);
    return DeviceResult::Ok;
}

DeviceResult EspGpio::portConfigure(uint32_t port, uint8_t mask, const GpioConfig* config)
{
    ContextLock lock(mutex);
    gpio_config_t gpioConfig;
    gpioConfig.pin_bit_mask = mask;
    gpioConfig.pin_bit_mask <<= port * 8; // Adjust the mask for the port
    if(gpioConfig.pin_bit_mask == 0)
        return DeviceResult::Ok;

    // Map GPIO mode from GpioConfig to gpio_config_t
    switch (config->mode) {
        case GPIO_CFG_MODE_DISABLE:
            gpioConfig.mode = GPIO_MODE_DISABLE;
            break;
        case GPIO_CFG_MODE_INPUT:
            gpioConfig.mode = GPIO_MODE_INPUT;
            break;
        case GPIO_CFG_MODE_OUTPUT:
            gpioConfig.mode = GPIO_MODE_OUTPUT;
            break;
        case GPIO_CFG_MODE_OUTPUT_OD:
            gpioConfig.mode = GPIO_MODE_OUTPUT_OD;
            break;
        case GPIO_CFG_MODE_INPUT_OUTPUT_OD:
            gpioConfig.mode = GPIO_MODE_INPUT_OUTPUT_OD;
            break;
        default:
            ESP_LOGE("GPIO", "Unsupported GPIO mode");
            return DeviceResult::NotSupported;
    }

    // Map GPIO pull-up and pull-down settings from GpioConfig to gpio_config_t
    switch (config->pull) {
        case GPIO_CFG_PULL_DISABLE:
            gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
            gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case GPIO_CFG_PULLUP_ENABLE:
            gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
            gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case GPIO_CFG_PULLDOWN_ENABLE:
            gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
            gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        default:
            ESP_LOGE("GPIO", "Unsupported GPIO pull configuration");
            return DeviceResult::NotSupported;
    }

    // Map GPIO interrupt type from GpioConfig to gpio_config_t
    switch (config->intr) {
        case GPIO_CFG_INTR_DISABLE:
            gpioConfig.intr_type = GPIO_INTR_DISABLE;
            break;
        case GPIO_CFG_INTR_POSEDGE:
            gpioConfig.intr_type = GPIO_INTR_POSEDGE;
            break;
        case GPIO_CFG_INTR_NEGEDGE:
            gpioConfig.intr_type = GPIO_INTR_NEGEDGE;
            break;
        case GPIO_CFG_INTR_ANYEDGE:
            gpioConfig.intr_type = GPIO_INTR_ANYEDGE;
            break;
        case GPIO_CFG_INTR_LOW_LEVEL:
            gpioConfig.intr_type = GPIO_INTR_LOW_LEVEL;
            break;
        case GPIO_CFG_INTR_HIGH_LEVEL:
            gpioConfig.intr_type = GPIO_INTR_HIGH_LEVEL;
            break;
        default:
            ESP_LOGE("GPIO", "Unsupported GPIO interrupt configuration");
            return DeviceResult::NotSupported;
    }

    // Configure GPIO with the specified settings
    esp_err_t err = gpio_config(&gpioConfig);
    if (err != ESP_OK) {
        ESP_LOGE("GPIO", "Failed to configure GPIO: %s", esp_err_to_name(err));
        return DeviceResult::Error;
    }
    return DeviceResult::Ok;
}


DeviceResult EspGpio::portIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback)
{
    ContextLock lock(mutex);
    gpio_num_t gpioNum = static_cast<gpio_num_t>(port * 8 + pin);

    // Register ISR handler for the GPIO pin
    if (gpio_isr_handler_add(gpioNum, gpio_isr_handler, (void*)&callback) != ESP_OK) {
        return DeviceResult::Error; // Return error if registration fails
    }

    return DeviceResult::Ok;
}

DeviceResult EspGpio::portIsrRemoveCallback(uint32_t port, uint8_t pin)
{
    ContextLock lock(mutex);
    gpio_num_t gpioNum = static_cast<gpio_num_t>(port * 8 + pin);

    // Remove ISR handler for the GPIO pin
    if (gpio_isr_handler_remove(gpioNum) != ESP_OK) {
        return DeviceResult::Error; // Return error if removal fails
    }

    return DeviceResult::Ok;
}

void IRAM_ATTR EspGpio::gpio_isr_handler(void* arg)
{
    std::function<void()>* callback = reinterpret_cast<std::function<void()>*>(arg);
    (*callback)(); // Call the registered callback function
}

