#include "MAX14830Uart.h"

DeviceResult MAX14830Uart::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("maxDevice", &maxDeviceKey),  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Missing parameter: maxDevice");
	DeviceSetStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult MAX14830Uart::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(maxDevice, deviceManager->getDeviceByKey<MAX14830>(maxDeviceKey), DeviceStatus::Dependencies, DeviceResult::Error, TAG, "Missing dependency: maxDevice");
	DeviceSetStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}

DeviceResult MAX14830Uart::DeviceInit()
{
	ContextLock lock(mutex);
	


	// Tell the driver the device is initialized and ready to use.
	DeviceSetStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult MAX14830Uart::UartConfigure(const UartConfig *config)
{




    return DeviceResult();
}

DeviceResult MAX14830Uart::StreamWrite(const uint8_t *data, size_t length, size_t *written, TickType_t timeout)
{
    return DeviceResult();
}

DeviceResult MAX14830Uart::StreamRead(uint8_t *data, size_t length, size_t *read, TickType_t timeout)
{
    return DeviceResult();
}
