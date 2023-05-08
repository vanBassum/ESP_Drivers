#pragma once
#include "esp_system.h"



class IGPIO
{
public:
	virtual void SetInputs(uint32_t bank, uint32_t pins) = 0;
	virtual void SetOutputs(uint32_t bank, uint32_t pins) = 0;
	virtual void WritePins(uint32_t bank, uint32_t pins) = 0;
	virtual uint64_t ReadPins(uint32_t bank, uint32_t pins) = 0;
	virtual uint64_t WaitForChange(uint32_t bank, uint32_t pins) = 0;
};




