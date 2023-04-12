#pragma once

#include "esp_system.h"
#include "freertos/freertos.h"
#include "freertos/queue.h"




namespace ESP_Base
{
	/*
	template<typename T>
	class Queue
	{
		QueueHandle_t handle = NULL;
	public:	
		~Queue()
		{
			if (handle != NULL)
				vStreamBufferDelete(handle);
		}
		
		///queueLength = The maximum number of items the queue can hold at any one time.
		bool Init(size_t queueLength)
		{
			if (handle != NULL)
				vQueueDelete(handle);
			handle = xQueueCreate(queueLength, sizeof(T));
			return handle != NULL;
		}
		
				
		bool Receive(T* data)
		{
			if (handle == NULL) return false;
			return xQueueReceive(handle, data, portMAX_DELAY) == pdTRUE;
		}
		
		bool SendToBack(const T* data)
		{
			if (handle == NULL) return false;
			return xQueueSendToBack(handle, data, portMAX_DELAY) == pdTRUE;

		}
		
		bool SendToFront(const T* data)
		{
			if (handle == NULL) return false;
			return xQueueSendToFront(handle, data, portMAX_DELAY) == pdTRUE;

		}
		
	};
*/
}



