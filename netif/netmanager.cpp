#include "netmanager.h"

bool NetManager::Init()
{
	esp_err_t ret = esp_netif_init();

	if (ret == ESP_OK)
		ret = esp_event_loop_create_default();
	return ret == ESP_OK;	
}

bool NetManager::Config(esp_netif_t *handle, const esp_netif_config_t *config1)
{
	//esp_netif_t *eth_netif = esp_netif_new(&esp_netif_config); // create network interface for driver
	//esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
	esp_netif_t *eth_netif = NULL;
	
	eth_netif = esp_netif_new(config1);
	handle = eth_netif;

	if (handle != NULL) return ESP_OK;
	else return 0;
}




