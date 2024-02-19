#pragma once
#include "interfaces.h"	
#include "DeviceManager.h"
#include "MAX14830.h"

class MAX14830Gpio : public IGpio
{
    constexpr static const char* TAG = "MAX14830Gpio";
	Mutex mutex;

	//Dependencies:
    const char* maxDeviceKey = nullptr;		
    std::shared_ptr<MAX14830> maxDevice;	

public:
	virtual DeviceResult setDeviceConfig(IDeviceConfig& config) override;
    virtual DeviceResult loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult init() override;

    virtual DeviceResult portConfigure(uint32_t port, uint8_t mask, const GpioConfig* config) override;
    virtual DeviceResult portRead(uint32_t port, uint8_t mask, uint8_t *value) override;
    virtual DeviceResult portWrite(uint32_t port, uint8_t mask, uint8_t value) override;
    DeviceResult portIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback);
    DeviceResult portIsrRemoveCallback(uint32_t port, uint8_t pin);

};

