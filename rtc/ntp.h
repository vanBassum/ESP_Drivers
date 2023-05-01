#pragma once
#include "esp_sntp.h"
#include "esp_base.h"
#include <string>

class NTP
{
	static void OnSyncCallback(struct timeval* tv);
	
public:
	static Event<struct timeval*> OnSync; 
	static bool Init(const std::string host);
	static void SetSystemTime(DateTime* value);
	static void SetTimeZone(std::string timeZone);
};


