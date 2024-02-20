#include "MAX14830Uart.h"
#include "max310x.h"


DeviceResult MAX14830Uart::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("maxDevice", &maxDeviceKey),  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Missing parameter: maxDevice");
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("port", &port),  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Missing parameter: port");
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
	uint8_t flowCtrlRegVal = 0;

	//TODO: Setup things like ISR 


	// Tell the driver the device is initialized and ready to use.
	DeviceSetStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult MAX14830Uart::UartConfigure(const UartConfig *config)
{
	ContextLock lock(mutex);
	return maxDevice->UartConfigure(port, config);
}

DeviceResult MAX14830Uart::StreamWrite(const uint8_t *data, size_t txLen, size_t *written, TickType_t timeout)
{
	ContextLock lock(mutex);
	return maxDevice->StreamWrite(port, data, txLen, written, timeout);
}

DeviceResult MAX14830Uart::StreamRead(uint8_t *data, size_t length, size_t *read, TickType_t timeout)
{
	ContextLock lock(mutex);
	return maxDevice->StreamRead(port, data, length, read, timeout);
}

