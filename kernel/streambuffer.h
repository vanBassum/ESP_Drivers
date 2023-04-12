#pragma once

#include "interfaces/istream.h"
#include "freertos/stream_buffer.h"


class StreamBuffer : public IStream
{
	StreamBufferHandle_t handle = NULL;
		
public:	
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
		
	size_t Read(void* data, size_t size) override
	{
		if (handle == NULL) return 0;
		return xStreamBufferReceive(handle, data, size, portMAX_DELAY);
	}
		
	size_t Write(const void* data, size_t size) override
	{
		if (handle == NULL) return 0;
		size_t result = xStreamBufferSend(handle, data, size, portMAX_DELAY);
		OnDataReady.Invoke(this);
		return result;
	}
};


