#include "wifi.h"
#include "esp_wifi.h"
#include "esp_base.h"

const char* TAG	= "wifi";

bool Wifi::Init()
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	INIT_OR_RETURN(TAG, "Init WIFI", esp_wifi_init(&cfg));
	
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);
	
	return false;
}


void Wifi::Connect(const char* ssid, const char* pwd)
{
}

