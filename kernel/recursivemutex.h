#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"


class RecursiveMutex
{
	SemaphoreHandle_t handle = NULL;
	const char* msg = NULL;
public:
	RecursiveMutex(const char* msg = NULL)
		: msg(msg)
	{
		if(msg != NULL)
			ESP_LOGI("RecursiveMutex", "Create %s", msg);
		handle = xSemaphoreCreateRecursiveMutex();
	}
		
	~RecursiveMutex()
	{
		if (msg != NULL)
			ESP_LOGI("RecursiveMutex", "Destruct %s", msg);
		if(handle != NULL)
			vSemaphoreDelete(handle);
	}
		
	bool Take(int timeout = portMAX_DELAY) const
	{
		return xSemaphoreTakeRecursive(handle, timeout) == pdTRUE;
	}

	bool Give() const
	{
		return xSemaphoreGiveRecursive(handle) == pdTRUE;
	}

	class ContextLock
	{
		const RecursiveMutex& mutex;
		const char* msg = NULL;
	public:
		ContextLock(const RecursiveMutex& mutex, const char* msg = NULL)
			: mutex(mutex)
			, msg(msg)
		{
			if (mutex.handle == NULL && msg != NULL)
				ESP_LOGE("ContextLock", "NULL  %s", msg);
			
			if (msg != NULL && mutex.msg != NULL)
				ESP_LOGI("ContextLock", "TAKE  %s %s", msg, mutex.msg);
			else if (msg != NULL)
				ESP_LOGI("ContextLock", "TAKE  %s", msg);
			mutex.Take();
			
			if (msg != NULL && mutex.msg != NULL)
				ESP_LOGI("ContextLock", "TAKEN %s %s", msg, mutex.msg);
			else if (msg != NULL)
				ESP_LOGI("ContextLock", "TAKEN %s", msg);

		}
	
		~ContextLock()
		{
			if (msg != NULL && mutex.msg != NULL)
				ESP_LOGI("ContextLock", "GIVE  %s %s", msg, mutex.msg);
			else if (msg != NULL)
				ESP_LOGI("ContextLock", "GIVE  %s", msg);
			mutex.Give();
			if (msg != NULL && mutex.msg != NULL)
				ESP_LOGI("ContextLock", "GIVEN %s %s", msg, mutex.msg);
			else if (msg != NULL)
				ESP_LOGI("ContextLock", "GIVEN %s", msg);
		}
	
	};
};
