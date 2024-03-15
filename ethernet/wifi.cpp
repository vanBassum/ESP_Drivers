#include "wifi.h"


static const char *TAG	= "Wifi";

//static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
//{
//	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
//		esp_wifi_connect();
//	}
//	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
//		esp_wifi_connect();	//Retry connecting.
//	}
//	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
//
//	}
//}


// Wifi::Wifi(NetManager& netManager)
// {
// 	wifi_init_config_t cfg2 = WIFI_INIT_CONFIG_DEFAULT();
	
// 	netIF = esp_netif_create_default_wifi_sta();
// 	ESP_ERROR_CHECK(esp_wifi_init(&cfg2));
// 	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); 

// }


bool Wifi::Connect(const std::string& ssid, const std::string& pwd)
{
	wifi_config_t wifi_config = { };
	memcpy(wifi_config.sta.ssid, ssid.c_str(), ssid.size());
	memcpy(wifi_config.sta.password, pwd.c_str(), pwd.size());
	
	INIT_OR_RETURN(TAG, "Set WIFI mode", esp_wifi_set_mode(WIFI_MODE_STA) == ESP_OK);
	INIT_OR_RETURN(TAG, "Set WIFI config", esp_wifi_set_config(WIFI_IF_STA, &wifi_config) == ESP_OK);
	INIT_OR_RETURN(TAG, "Start WIFI", esp_wifi_start() == ESP_OK);
	INIT_OR_RETURN(TAG, "Connect WIFI", esp_wifi_connect() == ESP_OK);
	
	return true;
}

