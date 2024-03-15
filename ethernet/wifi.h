#pragma once

#include "esp_wifi.h"
#include "esp_base.h"
#include <string>
#include "netif.h"

class Wifi : public NetIF
{
	const char* TAG = "Wifi";
public:
	
	//Wifi(NetManager& netManager);
		
	bool Connect(const std::string& ssid, const std::string& pwd);

};

