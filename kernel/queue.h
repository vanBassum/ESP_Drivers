#pragma once
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

template<typename T>
class Queue
{
	QueueHandle_t handle = NULL;
		
public:	
	~Queue()
	{
		if (handle != NULL)
			vQueueDelete(handle);
	}
		
	bool Init(size_t length)
	{
		if (handle != NULL)
			vQueueDelete(handle);
		handle = xQueueCreate(length, sizeof(T));
		return handle != NULL;
	}
	
	bool Send(const T& item, TickType_t timeout = portMAX_DELAY)
	{
		return xQueueSend(handle, (void *) &item, timeout) == pdPASS;
	}
	
	bool Receive(T& item, TickType_t timeout = portMAX_DELAY)
	{
		return xQueueReceive(handle, (void *) &item, timeout) == pdPASS;
	}
	
};


