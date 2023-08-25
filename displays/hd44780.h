#pragma once
#include "esp_system.h"
#include "mcp23s17.h"
#include <string>
//https://github.com/Matiasus/HD44780/blob/master/lib/hd44780.c


class HD44780
{
public:
	enum class Pins
	{
		NONE = 0x0000,
		RW   = 0x0001,
		RS   = 0x0002,
		E    = 0x0004,
		BL   = 0x0008,
		DB0  = 0x0010,
		DB1  = 0x0020,
		DB2  = 0x0040,
		DB3  = 0x0080,
		DB4  = 0x0100,
		DB5  = 0x0200,
		DB6  = 0x0400,
		DB7  = 0x0800,
		ALL  = 0x0FFF,
	};
	
	enum class PinModes
	{
		PIN_INPUT,
		PIN_OUTPUT,
	};
	
	class IGPIO
	{
	public:
		virtual void SetPinsMode(Pins mask, PinModes mode)	= 0;
		virtual void SetPins(Pins mask, Pins value)	= 0;
		virtual void GetPins(Pins mask, Pins* value) = 0;
	};
	
private:
	IGPIO& io;
	const char* TAG = "HD44780";
	void SetCursor(int x, int row);
	void LCD_cmd(unsigned char cmd);
	void WaitBFClear();
	void LCD_Data(unsigned char cmd);
		
public:
	HD44780(IGPIO& io);
	void SetBacklight(bool enabled);
	void Write(std::string message);
	void Write(std::string message, int x, int y);
};


DEFINE_ENUM_CLASS_FLAG_OPERATORS(HD44780::Pins, uint32_t);


