#pragma once
#include "esp_netif.h"
#include "esp_event.h"

class NetManager
{
public:	
	NetManager()
	{
		static bool inited = false;
		if (!inited)
		{
			esp_err_t ret = esp_netif_init();

			if (ret == ESP_OK)
				ret = esp_event_loop_create_default();
			inited = true;
		}
	}
};

