#pragma once
#include "esp_sntp.h"
#include "esp_base.h"
#include <string>

class NTP
{
	static void OnSyncCallback(struct timeval* tv);
	static bool NtpReceived;
public:
	
	static Event<void, struct timeval*> OnSync; 
	static bool Init(const std::string& host);
	static void SetSystemTime(DateTime* value);
	static void SetTimeZone(const std::string& timeZone);
	static bool Received();
};


