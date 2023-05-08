#pragma once

#include "freertos/stream_buffer.h"
#include "esp_base.h"

class StreamBuffer
{
	StreamBufferHandle_t handle = NULL;
		
public:	
	Event<StreamBuffer*> OnDataReady;
	
	~StreamBuffer()
	{
		if (handle != NULL)
			vStreamBufferDelete(handle);
	}
		
	bool Init(size_t xBufferSizeBytes, size_t xTriggerLevelBytes)
	{
		if(handle != NULL)
			vStreamBufferDelete(handle);
		handle = xStreamBufferCreate(xBufferSizeBytes, xTriggerLevelBytes);
		return handle != NULL;
	}
		
	size_t GetAvailableBytes()
	{
		if (handle == NULL) return 0;
		return xStreamBufferBytesAvailable(handle);
	}
		
	size_t GetAvailableSpace()
	{
		if (handle == NULL) return 0;
		return xStreamBufferSpacesAvailable(handle);
	}
		
	size_t Read(void* data, size_t size)
	{
		if (handle == NULL) return 0;
		return xStreamBufferReceive(handle, data, size, portMAX_DELAY);
	}
		
	size_t Write(const void* data, size_t size)
	{
		if (handle == NULL) return 0;
		size_t result = xStreamBufferSend(handle, data, size, portMAX_DELAY);
		OnDataReady.Invoke(this);
		return result;
	}
};


