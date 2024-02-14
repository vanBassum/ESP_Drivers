#pragma once
#include <functional>
#include "deviceConfig.h"




class IDeviceDetector
{
public:
    virtual void search(std::function<void(IDeviceConfig& deviceConfig)> foundCallback) = 0;
    virtual ~IDeviceDetector() {}
};
