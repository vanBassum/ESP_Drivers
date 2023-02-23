#pragma once
#include "esp_system.h"

namespace ESP_Drivers
{
	class GLCD
	{
	public:
		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;
		virtual void WriteRow(uint32_t y, uint8_t* data, size_t size) = 0;
	};
}