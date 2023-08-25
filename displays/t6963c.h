#pragma once
#include "mcp23s17.h"


class T6963C
{
	const char* TAG = "T6963C";
public:
	struct Settings
	{
		uint16_t width = 240;
		uint16_t height = 64;
	};
	
	enum class Pins
	{
		NONE = 0x0000,
		WR   = 0x0001,
		RD   = 0x0002,
		CE    = 0x0004,
		CD   = 0x0008,
		RST  = 0x0010,
		
		DB0  = 0x0100,
		DB1  = 0x0200,
		DB2  = 0x0400,
		DB3  = 0x0800,
		DB4  = 0x1000,
		DB5  = 0x2000,
		DB6  = 0x4000,
		DB7  = 0x8000,
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
		virtual void ConsecutivePinWriting(Pins mask, Pins* states, size_t length) = 0;
	};
private:
	IGPIO& io;
	const Settings& settings;
	uint8_t rows = 0;
	uint8_t columns = 0;
	void WriteByte(bool cd, uint8_t data);
	void SetCtrlPins(bool wr, bool rd, bool cd, bool ce);
	void SetDataPins(uint8_t data);
	void WriteCmd(uint8_t cmd);
	void WriteCmd(uint8_t cmd, uint8_t data1);
	void WriteCmd(uint8_t cmd, uint8_t data1, uint8_t data2);
	void Clear();
	void SetAddress(uint8_t col, uint8_t row);
	void SetCursor(uint8_t col, uint8_t row);
	void StartAutoWrite();
	void StopAutoWrite();
	void AutoWrite(uint8_t data);
	void AutoWriteChar(char data);
	void StatusCheck() {}	
	void OSet(uint8_t data, bool wr, bool rd, bool cd, bool ce, Pins* order);
	Pins* OWriteByte(bool cd, uint8_t data, Pins* order);
	Pins* OWriteCmd(uint8_t cmd, uint8_t data1, Pins* order);
	Pins* OWriteCmd(uint8_t cmd, uint8_t data1, uint8_t data2, Pins* order);
	Pins* OWriteCmd(uint8_t cmd, Pins* order);
	Pins* OSetAddress(uint8_t col, uint8_t row, Pins* order);
	
public:
	T6963C(IGPIO& io, const Settings& settings);
	void WriteRow(uint32_t y, uint8_t* data, size_t size);
};

DEFINE_ENUM_CLASS_FLAG_OPERATORS(T6963C::Pins, uint32_t);
