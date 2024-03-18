#pragma once

#include "esp_wifi.h"
#include "esp_base.h"
#include "netif.h"

class Wifi : public IDevice
{
	constexpr static const char* TAG = "Wifi";
    
    Mutex mutex;

    // Dependencies:
    const char* netIfDeviceKey = nullptr;
    std::shared_ptr<NetIF> netIfDevice;
    
    static void Wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

public:
    virtual Result DeviceSetConfig(IDeviceConfig &config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit();

	Result Connect(const std::string& ssid, const std::string& pwd);
};

