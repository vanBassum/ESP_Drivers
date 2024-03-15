#pragma once
#include "esp_base.h"
#include "esp_netif.h"

class NetIF : public IDevice
{
    constexpr static const char *TAG = "NetIF";
    Mutex mutex;

    // Dependencies:

    //const char* netIfDeviceKey = nullptr;
    //std::shared_ptr<NetIF> netIfDevice;
	
protected:
	esp_netif_t *netIF;
	
public:
	virtual ~NetIF() {}

    //virtual Result DeviceSetConfig(IDeviceConfig &config) = 0;
    //virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) = 0;
    virtual Result DeviceInit() override;
};


