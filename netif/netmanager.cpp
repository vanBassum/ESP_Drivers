#include "netmanager.h"

bool NetManager::Init()
{
	esp_err_t ret = esp_netif_init();

	if (ret == ESP_OK)
		ret = esp_event_loop_create_default();
	return ret == ESP_OK;	
}



