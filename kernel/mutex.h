#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

class Mutex
{
	const char* TAG = "Mutex";
	SemaphoreHandle_t handle = NULL;
public:
	Mutex()
	{
		handle = xSemaphoreCreateMutex();
	}
		
	~Mutex()
	{
		if (handle != NULL)
			vSemaphoreDelete(handle);
	}
		
	bool Take(int timeout = portMAX_DELAY)
	{
		bool suc = xSemaphoreTake(handle, timeout) == pdTRUE;
		if (!suc)
		{
			
			ESP_LOGI(TAG, "Error while taking mutex");
		}
		return suc;
	}

	bool Give()
	{
		return xSemaphoreGive(handle) == pdTRUE;
	}

};