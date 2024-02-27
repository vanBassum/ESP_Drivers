#pragma once
#include "datetime.h"
#include "IDevice.h"

class IRtc : public IDevice
{
public:

    virtual ~IRtc() {}

    virtual Result DeviceSetConfig(IDeviceConfig& config) = 0;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) = 0;
    virtual Result DeviceInit() = 0;

    virtual Result TimeGet(DateTime& value)  { return Result::NotSupported; };
	virtual Result TimeSet(DateTime& value)  { return Result::NotSupported; };
};