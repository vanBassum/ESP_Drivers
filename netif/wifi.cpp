#include "wifi.h"
#include "esp_wifi.h"
#include "esp_base.h"


const char* TAG	= "Wifi";

bool Wifi::Init(NetManager* netManager)
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	INIT_OR_RETURN(TAG, "Init", esp_wifi_init(&cfg) == ESP_OK);
	INIT_OR_RETURN(TAG, "Set storage RAM", esp_wifi_set_storage(WIFI_STORAGE_RAM) == ESP_OK);
	
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	INIT_OR_RETURN(TAG, "Create STA", sta_netif != NULL);
	
	return true;
}


bool Wifi::Connect(const std::string ssid, const std::string pass)
{
	//ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	int ssidLen = ssid.size();
	int passLen = pass.size();
	
	wifi_config_t wifi_config;
	ssid.copy((char*)wifi_config.sta.ssid, ssidLen, 0);
	ssid.copy((char*)wifi_config.sta.password, passLen, 0);
	
	INIT_OR_RETURN(TAG, "Set WIFI mode", esp_wifi_set_mode(WIFI_MODE_STA) == ESP_OK);
	INIT_OR_RETURN(TAG, "Set WIFI config", esp_wifi_set_config(WIFI_IF_STA, &wifi_config) == ESP_OK);
	INIT_OR_RETURN(TAG, "Start WIFI", esp_wifi_start() == ESP_OK);
	INIT_OR_RETURN(TAG, "Connect WIFI", esp_wifi_connect() == ESP_OK);
	
	return true;
}

