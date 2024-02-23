#include "espgpio.h"

// Initialize static member variable
std::list<std::shared_ptr<EspGpio::IsrHandle>> EspGpio::callbacks;

Result EspGpio::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result EspGpio::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
    DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}


Result EspGpio::DeviceInit()
{
	ContextLock lock(mutex);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_SHARED);
	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
}

Result EspGpio::GpioConfigure(uint32_t port, uint8_t mask, const GpioConfig* config)
{
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
    gpio_config_t gpioConfig;
    gpioConfig.pin_bit_mask = mask;
    gpioConfig.pin_bit_mask <<= port * 8; // Adjust the mask for the port
    if(gpioConfig.pin_bit_mask == 0)
        return Result::Ok;

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
            return Result::NotSupported;
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
            return Result::NotSupported;
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
            return Result::NotSupported;
    }

    // Configure GPIO with the specified settings
    esp_err_t err = gpio_config(&gpioConfig);
    if (err != ESP_OK) {
        ESP_LOGE("GPIO", "Failed to configure GPIO: %s", esp_err_to_name(err));
        return Result::Error;
    }
    return Result::Ok;
}

Result EspGpio::GpioRead(uint32_t port, uint8_t mask, uint8_t* value)      
{
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
    *value = 0x00;
    if(mask & 0x01) *value += gpio_get_level(static_cast<gpio_num_t>(port * 8 + 0)) * 0x01;
    if(mask & 0x02) *value += gpio_get_level(static_cast<gpio_num_t>(port * 8 + 1)) * 0x02;
    if(mask & 0x04) *value += gpio_get_level(static_cast<gpio_num_t>(port * 8 + 2)) * 0x04;
    if(mask & 0x08) *value += gpio_get_level(static_cast<gpio_num_t>(port * 8 + 3)) * 0x08;
    if(mask & 0x10) *value += gpio_get_level(static_cast<gpio_num_t>(port * 8 + 4)) * 0x10;
    if(mask & 0x20) *value += gpio_get_level(static_cast<gpio_num_t>(port * 8 + 5)) * 0x20;
    if(mask & 0x40) *value += gpio_get_level(static_cast<gpio_num_t>(port * 8 + 6)) * 0x40;
    if(mask & 0x80) *value += gpio_get_level(static_cast<gpio_num_t>(port * 8 + 7)) * 0x80;
    return Result::Ok;
}

Result EspGpio::GpioWrite(uint32_t port, uint8_t mask, uint8_t value)      
{
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
    if(mask & 0x01) gpio_set_level(static_cast<gpio_num_t>(port * 8 + 0), value & 0x01);
    if(mask & 0x02) gpio_set_level(static_cast<gpio_num_t>(port * 8 + 1), value & 0x02);
    if(mask & 0x04) gpio_set_level(static_cast<gpio_num_t>(port * 8 + 2), value & 0x04);
    if(mask & 0x08) gpio_set_level(static_cast<gpio_num_t>(port * 8 + 3), value & 0x08);
    if(mask & 0x10) gpio_set_level(static_cast<gpio_num_t>(port * 8 + 4), value & 0x10);
    if(mask & 0x20) gpio_set_level(static_cast<gpio_num_t>(port * 8 + 5), value & 0x20);
    if(mask & 0x40) gpio_set_level(static_cast<gpio_num_t>(port * 8 + 6), value & 0x40);
    if(mask & 0x80) gpio_set_level(static_cast<gpio_num_t>(port * 8 + 7), value & 0x80);
    return Result::Ok;
}



Result EspGpio::GpioIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback)
{
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
    gpio_num_t gpioNum = static_cast<gpio_num_t>(port * 8 + pin);

    // Store the callback in a member variable to ensure its lifetime.
    std::shared_ptr<IsrHandle> handle = std::make_shared<IsrHandle>();
    handle->device = this;
    handle->pin = gpioNum;
    handle->callback = callback;
    callbacks.push_back(handle);

    // Register ISR handler for the GPIO pin
    if (gpio_isr_handler_add(gpioNum, gpio_isr_handler, handle.get()) != ESP_OK) {
        callbacks.remove(handle); // Remove callback on failure
        return Result::Error; // Return error if registration fails
    }
    return Result::Ok;
}

Result EspGpio::GpioIsrRemoveCallback(uint32_t port, uint8_t pin)
{
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
    gpio_num_t gpioNum = static_cast<gpio_num_t>(port * 8 + pin);

    // Remove ISR handler for the GPIO pin
    if (gpio_isr_handler_remove(gpioNum) != ESP_OK) {
        return Result::Error; // Return error if removal fails
    }

    // Search for the callback associated with the GPIO pin and remove it from the list
    for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
        if ((*it)->device == this && (*it)->pin == gpioNum) {
            callbacks.erase(it);
            return Result::Ok; // Callback removed successfully
        }
    }

    // If the callback is not found, return an error
    return Result::Error;
}

void IRAM_ATTR EspGpio::gpio_isr_handler(void* arg)
{
    IsrHandle* handle = static_cast<IsrHandle*>(arg);

    // Ensure handle is not null
    if (handle != nullptr) {
        // Access the members of the IsrHandle object
        EspGpio* device = handle->device;
        gpio_num_t pin = handle->pin;
        std::function<void()> callback = handle->callback;

        // Check if the callback is valid before invoking it
        if (callback) {
            callback(); // Call the registered callback function
        }
    }
}


