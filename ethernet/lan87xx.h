#pragma once

#include "netmanager.h"
#include "esp_base.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "netif.h"

class Lan87xx : public NetIF
{	
public:
	
	Lan87xx(NetManager& netManager);
};

