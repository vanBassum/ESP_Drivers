#pragma once
#include "IDeviceDetector.h"

//Takes devicetree and initiates devices
class DeviceTreeDetector : public IDeviceDetector
{
	const DeviceTree deviceTree;

public:
	DeviceTreeDetector(const DeviceTree deviceTree) : deviceTree(deviceTree){ }

    void search(std::function<void(IDeviceConfig& deviceConfig)> foundCallback) override
    {
        for (int i = 0; deviceTree[i] != nullptr; i++) {
            DeviceConfigReader reader(deviceTree[i]);
            if(foundCallback)
                foundCallback(reader);
        }
    }
};


