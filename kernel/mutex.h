#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"


class IMutex
{
public:
	virtual ~IMutex() = default;
	virtual bool Take(TickType_t timeout = portMAX_DELAY) const = 0;
	virtual bool Give()  const =0 ;
};


class ContextLock
{
	constexpr const static char* TAG = "ContextLock";
	const IMutex& mutex;
	const char* msg = nullptr;
public:
	ContextLock(const IMutex& mutex, const char* msg = nullptr)
		: mutex(mutex)
		, msg(msg)
	{
		mutex.Take();
		if (msg != nullptr) ESP_LOGI(TAG, "Taken `%s`", msg);
	}
	
	~ContextLock()
	{
		if (msg != nullptr) ESP_LOGI(TAG, "Freeing `%s`", msg);
		mutex.Give();
	}
	
};


class Mutex : public IMutex
{
	const char* TAG = "Mutex";
	SemaphoreHandle_t handle = NULL;
public:
	Mutex()
	{
		handle = xSemaphoreCreateMutex();
	}
		
	~Mutex() override
	{
		if (handle != NULL)
			vSemaphoreDelete(handle);
	}
		
	bool Take(TickType_t timeout = portMAX_DELAY) const  override
	{
		return xSemaphoreTake(handle, timeout) == pdTRUE;
	}

	bool Give() const override
	{
		return xSemaphoreGive(handle) == pdTRUE;
	}

	bool IsTaken(void)   const
	{
		TaskHandle_t holderTask = xSemaphoreGetMutexHolder(handle);
		return holderTask != NULL;
	}
	

};



