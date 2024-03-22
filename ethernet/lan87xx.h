#pragma once
#include "esp_base.h"
#include "netif.h"

class LAN87xx : public IDevice
{
    constexpr static const char* TAG = "Lan87xx";

    Mutex mutex;

    // Dependencies:
    const char* netIfDeviceKey = nullptr;
    std::shared_ptr<NetIF> netIfDevice;
    esp_netif_t *eth_netif;  
    
    static void eth_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

public:
    virtual Result DeviceSetConfig(IDeviceConfig &config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit();

    Result GetIpInfo(esp_netif_ip_info_t* ip_info);
    Result SetDNS(esp_ip4_addr_t ip, esp_netif_dns_type_t type);
    Result SetStaticIp(esp_netif_ip_info_t ip);
    Result SetDHCP();
};
