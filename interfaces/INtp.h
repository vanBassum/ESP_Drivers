#pragma once
#include "datetime.h"
#include "IDevice.h"

class INtp : public IDevice
{
public:

    virtual ~INtp() {}

    virtual Result DeviceSetConfig(IDeviceConfig& config) = 0;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) = 0;
    virtual Result DeviceInit() = 0;

    virtual Result NtpOnSyncSetCallback(std::function<void(DateTime newTime)> callback) = 0;
};
