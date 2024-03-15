#include "netif.h"
#include "esp_event.h"

Result NetIF::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
    // RETURN_ON_ERR(config.getProperty("server", &server));
	DeviceSetStatus(DeviceStatus::Dependencies);
    return Result::Ok;
}

Result NetIF::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
    ContextLock lock(mutex);
    DeviceSetStatus(DeviceStatus::Initializing);
    return Result::Ok;
}

Result NetIF::DeviceInit()
{
	ContextLock lock(mutex);
	
	if((DeviceCheckStatus(DeviceStatus::Configuring)) == Result::Ok)
	{
		esp_err_t err = 0;
		err = esp_netif_init();

		if(err == ESP_OK) err = esp_event_loop_create_default();
		if(err != ESP_OK) return Result::Error;
	}
   	DeviceSetStatus(DeviceStatus::Ready);
    return Result::Ok;
}

/* 	void NetIF::GetIpInfo(esp_netif_ip_info_t* ip_info)
	{
		esp_netif_get_ip_info(netIF, ip_info);
	}
	
	void NetIF::SetDNS(esp_ip4_addr_t ip, esp_netif_dns_type_t type)
	{
		//assert(false && "Doenst compile anymore, fix this");
		esp_netif_dns_info_t dns;
		dns.ip.u_addr.ip4.addr = ip.addr;
		//dns.ip.type = IPADDR_TYPE_V4;
		dns.ip.type = type;
		ESP_ERROR_CHECK(esp_netif_set_dns_info(netIF, type, &dns));
	}
	
	void NetIF::SetStaticIp(esp_netif_ip_info_t ip)
	{
		if (esp_netif_dhcpc_stop(netIF) != ESP_OK) {
			ESP_LOGE(TAG, "Failed to stop dhcp client");
			return;
		}

		if (esp_netif_set_ip_info(netIF, &ip) != ESP_OK) {
			ESP_LOGE(TAG, "Failed to set ip info");
			return;
		}
		ESP_LOGD(TAG, "Success to set static ip " IPSTR " " IPSTR " " IPSTR " ", IP2STR(&ip.ip), IP2STR(&ip.gw), IP2STR(&ip.netmask));
	}
	
	void NetIF::SetDHCP()
	{
		if (esp_netif_dhcpc_start(netIF) != ESP_OK) {
			ESP_LOGE(TAG, "Failed to start dhcp client");
			return;
		}
	} */