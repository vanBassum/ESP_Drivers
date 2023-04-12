#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace ESP_Base
{
	class RecursiveMutex
	{
		SemaphoreHandle_t handle = NULL;
	public:
		RecursiveMutex()
		{
			handle = xSemaphoreCreateRecursiveMutex();
		}
		
		~RecursiveMutex()
		{
			if(handle != NULL)
				vSemaphoreDelete(handle);
		}
		
		bool Take(int timeout = portMAX_DELAY)
		{
			return xSemaphoreTakeRecursive(handle, timeout) == pdTRUE;
		}

		bool Give()
		{
			return xSemaphoreGiveRecursive(handle) == pdTRUE;
		}

	};
	
	
}