#include "MAX14830Uart.h"
#include "max310x.h"


Result MAX14830Uart::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	
	RETURN_ON_ERR(config.getProperty("maxDevice", &maxDeviceKey));
	RETURN_ON_ERR(config.getProperty("port", &port));
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result MAX14830Uart::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(deviceManager->getDeviceByKey<MAX14830>(maxDeviceKey, maxDevice));
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result MAX14830Uart::DeviceInit()
{
	ContextLock lock(mutex);
	uint8_t flowCtrlRegVal = 0;

	//TODO: Setup things like ISR 


	// Tell the driver the device is initialized and ready to use.
	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
}

Result MAX14830Uart::UartConfigure(const UartConfig *config)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
	return maxDevice->UartConfigure(port, config);
}

Result MAX14830Uart::StreamWrite(const uint8_t *data, size_t txLen, size_t *written, TickType_t timeout)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
	return maxDevice->StreamWrite(port, data, txLen, written, timeout);
}

Result MAX14830Uart::StreamRead(uint8_t *data, size_t length, size_t *read, TickType_t timeout)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device '%s' not ready", key);
	return maxDevice->StreamRead(port, data, length, read, timeout);
}

