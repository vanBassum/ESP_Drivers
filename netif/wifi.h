#pragma once
#include "netmanager.h"
#include "esp_wifi.h"
#include "esp_base.h"
#include <string>

class Wifi
{
	esp_netif_t *sta_netif;
public:
	
	bool Init(NetManager* netManager);
	
	bool Connect(const std::string& ssid, const std::string& pwd);
	
	void GetIpInfo(esp_netif_ip_info_t* ip_info)
	{
		esp_netif_get_ip_info(sta_netif, ip_info);
	}
	
};

