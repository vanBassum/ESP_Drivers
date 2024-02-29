#pragma once
//#include "esp_system.h"
//#include "mcp23s17.h"
#include <string>
//#include "gpio.h"
//#include "spi/device.h"
#include "interfaces/IGpio.h"
#include "DeviceManager.h"
#include "IGpio.h"
#include "helpers.h"
//#include "gpiopin.h"

//https://github.com/Matiasus/HD44780/blob/master/lib/hd44780.c

class HD44780 : public IDevice
{
	constexpr static const char* TAG = "HD44780";
	Mutex mutex;

	//Dependencies:
    const char* gpioDeviceKey = nullptr;
    std::shared_ptr<IGpio> gpioDevice;

	uint8_t d0_Port = 0; 
	uint8_t d0_Pin = 0;
	uint8_t d1_Port = 0; 
	uint8_t d1_Pin = 0;
	uint8_t d2_Port = 0; 
	uint8_t d2_Pin = 0;
	uint8_t d3_Port = 0; 
	uint8_t d3_Pin = 0;
	uint8_t d4_Port = 0; 
	uint8_t d4_Pin = 0;
	uint8_t d5_Port = 0; 
	uint8_t d5_Pin = 0;
	uint8_t d6_Port = 0; 
	uint8_t d6_Pin = 0;
	uint8_t d7_Port = 0; 
	uint8_t d7_Pin = 0;
	uint8_t e_Port = 0;
	uint8_t e_Pin = 0;
	uint8_t bl_Port = 0;          
    uint8_t bl_Pin = 0;
    uint8_t rs_Port = 0;
	uint8_t rs_Pin = 0;
	uint8_t d_Port = 0;

	uint8_t ALL = 0xFF;
	uint8_t NONE = 0;

	void SetCursor(int x, int row);
	void LCD_cmd(unsigned char cmd);
	void WaitBFClear();
	void LCD_Data(unsigned char cmd);

public:
	virtual Result DeviceSetConfig(IDeviceConfig& config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;
	
	void SetBacklight(bool enabled);
	void Write(std::string message);
	void Write(std::string message, int x, int y);
};

//DEFINE_ENUM_CLASS_FLAG_OPERATORS(HD44780::Pins, uint32_t);


