#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace ESP_Base
{
	class Mutex
	{
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
			return xSemaphoreTake(handle, timeout) == pdTRUE;
		}

		bool Give()
		{
			return xSemaphoreGive(handle) == pdTRUE;
		}

	};
	
	
}