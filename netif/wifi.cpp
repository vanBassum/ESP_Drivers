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

bool Wifi::Init(NetManager* netManager)
{
	sta_netif = esp_netif_create_default_wifi_sta();
//	esp_netif_config_t cfg = ESP_NETIF_DEFAULT_WIFI_STA();
//	esp_netif_t *netif = esp_netif_new(&cfg);
//	netManager->Config(sta_netif, &cfg);
//	assert(sta_netif);
//	esp_netif_attach_wifi_station(sta_netif);
//	esp_wifi_set_default_wifi_sta_handlers();
	
	INIT_OR_RETURN(TAG, "Create STA", sta_netif != NULL);
	
	wifi_init_config_t cfg2 = WIFI_INIT_CONFIG_DEFAULT();
	INIT_OR_RETURN(TAG, "Init", esp_wifi_init(&cfg2) == ESP_OK);
	INIT_OR_RETURN(TAG, "Set storage RAM", esp_wifi_set_storage(WIFI_STORAGE_RAM) == ESP_OK);
	
	//sta_netif = esp_netif_create_default_wifi_sta();
	//INIT_OR_RETURN(TAG, "Create STA", sta_netif != NULL);
	
	//esp_event_handler_instance_t instance_any_id;
	//esp_event_handler_instance_t instance_got_ip;
	//ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
	//	ESP_EVENT_ANY_ID,
	//	&event_handler,
	//	NULL,
	//	&instance_any_id));
	//ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
	//	IP_EVENT_STA_GOT_IP,
	//	&event_handler,
	//	NULL,
	//	&instance_got_ip));
	
	
	return true;
}


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

