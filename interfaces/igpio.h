#pragma once
#include "esp_system.h"
#include "callbacks.h"


class IGPIO
{
public:
	
	enum Mode
	{
		Input,
		Output,
	};
	
	// <Bank, Changes>
	Event<IGPIO*, uint32_t, uint32_t> OnPinChange;
	
	virtual void SetMode(uint32_t bank, uint32_t mask, Mode mode) = 0;
	virtual void WritePins(uint32_t bank, uint32_t mask, uint32_t value) = 0;
	virtual uint32_t ReadPins(uint32_t bank, uint32_t mask) = 0;
};




