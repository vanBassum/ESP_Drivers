#pragma once
#include "esp_base.h"
#include "netif.h"
#include "DeviceManager.h"

class LAN87xx : public NetIF
{
    constexpr static const char *TAG = "Lan87xx";
    Mutex mutex;

    // Dependencies:
    const char* netIfDeviceKey = nullptr;
    std::shared_ptr<NetIF> netIfDevice;
    
public:
    virtual Result DeviceSetConfig(IDeviceConfig &config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit(IDeviceConfig &config);

	
};
