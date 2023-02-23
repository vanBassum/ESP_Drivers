#pragma once
#include "esp_system.h"
#include <string>
#include "esp_base.h"

using namespace ESP_Base;

namespace ESP_Drivers
{


	class ESP32
	{
	public:
		static esp_err_t InitNVS();
		static esp_err_t InitWIFI(const char* ssid, const char* pwd);
		static esp_err_t InitNTP(void(*OnSyncCallback)(struct timeval* tv) = NULL, std::string server = "pool.ntp.org", std::string timezone = "UTC-1UTC,M3.5.0,M10.5.0/3");
		static void SetSystemTime(DateTime* value);
	};
}