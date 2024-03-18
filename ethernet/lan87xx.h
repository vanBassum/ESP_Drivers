#pragma once
#include "esp_base.h"
#include "netif.h"
#include "DeviceManager.h"

class LAN87xx : public NetIF
{
    constexpr static const char* TAG = "Lan87xx";

    Mutex mutex;

    // Dependencies:
    const char* netIfDeviceKey = nullptr;
    std::shared_ptr<NetIF> netIfDevice;
    
    static void eth_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);


public:
    virtual Result DeviceSetConfig(IDeviceConfig &config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit(IDeviceConfig &config);

	
};
