#include "MAX14830Gpio.h"


DeviceResult MAX14830Gpio::setDeviceConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("maxDevice", &maxDeviceKey),  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Missing parameter: maxDevice");
    setStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult MAX14830Gpio::loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(maxDevice, deviceManager->getDeviceByKey<MAX14830>(maxDeviceKey), DeviceStatus::Dependencies, DeviceResult::Error, TAG, "Missing dependency: maxDevice");
	setStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}

DeviceResult MAX14830Gpio::init()
{
	ContextLock lock(mutex);

	// Tell the driver the device is initialized and ready to use.
	setStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult MAX14830Gpio::portConfigure(uint32_t port, uint8_t mask, const GpioConfig *config)
{
    ContextLock lock(mutex);
    switch (config->intr)
    {
    case GPIO_CFG_INTR_DISABLE: //TODO: Implement later!
        break;
    
    default:
        return DeviceResult::NotSupported;
    }

    DEV_RETURN_ON_ERROR_SILENT(maxDevice->SetPinsMode(port, mask, config->mode));

    switch (config->pull)
    {
    case GPIO_CFG_PULL_DISABLE:  //TODO: Implement later!
        break;
    
    default:
        return DeviceResult::NotSupported;
    }
    return DeviceResult::Ok;
}

DeviceResult MAX14830Gpio::portRead(uint32_t port, uint8_t mask, uint8_t* value)
{
    ContextLock lock(mutex);
    DEV_RETURN_ON_ERROR_SILENT(maxDevice->GetPins(port, mask, value));
    return DeviceResult::Ok;
}

DeviceResult MAX14830Gpio::portWrite(uint32_t port, uint8_t mask, uint8_t value)
{
    ContextLock lock(mutex);
    DEV_RETURN_ON_ERROR_SILENT(maxDevice->SetPins(port, mask, value));
    return DeviceResult::Ok;
}
