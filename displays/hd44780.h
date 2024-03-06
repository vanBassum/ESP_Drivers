#pragma once
#include <string>
#include "DeviceManager.h"
#include "mcp23s17.h"
#include "helpers.h"

//https://github.com/Matiasus/HD44780/blob/master/lib/hd44780.c

class HD44780 : public IDevice
{
	constexpr static const char* TAG = "HD44780";
	Mutex mutex;

	//Dependencies:
    const char* mcpDeviceKey = nullptr;
    std::shared_ptr<MCP23S17> mcpDevice;

	void SetCursor(int x, int row);
	Result LCD_cmd(unsigned char cmd);
	void WaitBFClear();
	Result LCD_Data(unsigned char cmd);

public:
	virtual Result DeviceSetConfig(IDeviceConfig& config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;
	
	Result SetBacklight(bool enabled);
	Result printf(const char* format, ...);
	Result printf(int x, int y, const char* format, ...);
};

