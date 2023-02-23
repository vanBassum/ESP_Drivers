#include "esp32.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_sntp.h"


esp_err_t ESP_Drivers::ESP32::InitNVS()
{
	return nvs_flash_init();
}


esp_err_t ESP_Drivers::ESP32::InitWIFI(const char* ssid, const char* pwd)
{
	esp_err_t result = ESP_FAIL;
	static const char *TAG = "WIFI";
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	
	INIT_OR_RETURN(TAG, "Init NET IF", esp_netif_init());
	INIT_OR_RETURN(TAG, "Create event loop", esp_event_loop_create_default());
	INIT_OR_RETURN(TAG, "Init WIFI", esp_wifi_init(&cfg));
	
	//ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
	//ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));
	
	// Initialize default station as network interface instance (esp-netif)
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);
	
	// Initialize and start WiFi
	wifi_config_t wifi_config = { };
	memcpy(wifi_config.sta.ssid, ssid, strlen(ssid));
	memcpy(wifi_config.sta.password, pwd, strlen(pwd));

	INIT_OR_RETURN(TAG, "Set WIFI mode", esp_wifi_set_mode(WIFI_MODE_STA));
	INIT_OR_RETURN(TAG, "Set WIFI config", esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	INIT_OR_RETURN(TAG, "Start WIFI", esp_wifi_start());
	INIT_OR_RETURN(TAG, "Connect WIFI", esp_wifi_connect());
	return ESP_OK;
}


esp_err_t ESP_Drivers::ESP32::InitNTP(void(*OnSyncCallback)(struct timeval* tv) /* = NULL */, std::string server /* = "pool.ntp.org" */, std::string timezone /* = "UTC-1UTC,M3.5.0,M10.5.0/3" */)
{
	setenv("TZ", timezone.c_str(), 1);
	tzset();
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, server.c_str());
	sntp_init();
	
	if (OnSyncCallback != NULL)
		sntp_set_time_sync_notification_cb(OnSyncCallback);
	
	return ESP_OK;
}


void ESP_Drivers::ESP32::SetSystemTime(DateTime* value)
{
	struct timeval now;
	value->Get(&now);
	settimeofday(&now, NULL);
}
