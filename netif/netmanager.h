#pragma once
#include "esp_netif.h"
#include "esp_event.h"

class NetManager
{
public:
	bool Init();	

	bool Config(esp_netif_t *handle, const esp_netif_config_t *config1);

};

