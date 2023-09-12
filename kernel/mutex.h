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
		
	bool Take(TickType_t timeout = portMAX_DELAY) const
	{
		bool suc = xSemaphoreTake(handle, timeout) == pdTRUE;
		if (!suc)
		{
			
			ESP_LOGI(TAG, "Error while taking mutex");
		}
		return suc;
	}

	bool Give()  const
	{
		return xSemaphoreGive(handle) == pdTRUE;
	}

	bool IsTaken(void)   const
	{
		TaskHandle_t holderTask = xSemaphoreGetMutexHolder(handle);
		return holderTask != NULL;
	}
};


class ScopedLock
{
	const Mutex& mutex;
public:
	ScopedLock(const Mutex& mutex)
		: mutex(mutex)
	{
		mutex.Take();
	}
	
	~ScopedLock()
	{
		mutex.Give();
	}
	
};
