#pragma once
#include "esp_system.h"

namespace ESP_Drivers
{
	class GLCD_Mono
	{
	public:
		virtual uint16_t GetWidth() = 0;
		virtual uint16_t GetHeight() = 0;
		virtual void WriteRow(uint32_t y, uint8_t* data, size_t size) = 0;
	};

	class GLCD_16
	{
	public:
		virtual uint16_t GetWidth() = 0;
		virtual uint16_t GetHeight() = 0;
		virtual void SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) = 0;
		virtual void WriteWindow(uint8_t* data, size_t size) = 0;
	};

}