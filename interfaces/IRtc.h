#pragma once
#include "datetime.h"
#include "IDevice.h"

class IRtc : public IDevice
{
public:

    virtual ~IRtc() {}

    virtual DeviceResult DeviceSetConfig(IDeviceConfig& config) = 0;
    virtual DeviceResult DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) = 0;
    virtual DeviceResult DeviceInit() = 0;

    virtual DeviceResult TimeGet(DateTime& value)  { return DeviceResult::NotSupported; };
	virtual DeviceResult TimeSet(DateTime& value)  { return DeviceResult::NotSupported; };
};