#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


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
	public:
		ContextLock(const RecursiveMutex& mutex)
			: mutex(mutex)
		{
			mutex.Take();
		}
	
		~ContextLock()
		{
			mutex.Give();
		}
	
	};
};
