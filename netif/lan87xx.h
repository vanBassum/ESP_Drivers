#pragma once

#include "netmanager.h"
#include "esp_base.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_eth.h"
#include "esp_event.h"

class Lan87xx
{	
	esp_netif_t* eth_netif;

public:
	
	Lan87xx(NetManager& netManager);

	void GetIpInfo(esp_netif_ip_info_t* ip_info)
	{
		esp_netif_get_ip_info(eth_netif, ip_info);
	}
	
};

