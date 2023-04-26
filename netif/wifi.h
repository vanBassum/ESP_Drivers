#pragma once
#include "netmanager.h"
#include <string>

class Wifi
{
	
public:
	
	bool Init(NetManager* netManager);
	
	bool Connect(const std::string ssid, const std::string pwd);
	
	
};

