#pragma once
#include "esp_base.h"
#include "esp_netif.h"

class NetIF : public IDevice
{
    constexpr static const char *TAG = "NetIF";
    Mutex mutex;

    // Dependencies:
    // none
	
protected:
	esp_netif_t *netIF;
	
public:
	virtual ~NetIF() {}

    virtual Result DeviceSetConfig(IDeviceConfig &config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    void GetIpInfo(esp_netif_ip_info_t* ip_info);
    void SetDNS(esp_ip4_addr_t ip, esp_netif_dns_type_t type);
    void SetStaticIp(esp_netif_ip_info_t ip);
    void SetDHCP();
};


