#pragma once
#include "esp_system.h"
#include "esp_base.h"



class IStream
{
public:
	Event<IStream*> OnDataReady;
	virtual size_t Write(const void* data, size_t size) = 0;	
	virtual size_t Read(void* data, size_t size) = 0;
		
};

