#pragma once
#include "mcp23s17.h"
#include "gpio.h"


class T6963C
{
	const char* TAG = "T6963C";
public:
	enum class Pins
	{
		NONE = 0x0000,
		DB0  = 0x0001,
		DB1  = 0x0002,
		DB2  = 0x0004,
		DB3  = 0x0008,
		DB4  = 0x0010,
		DB5  = 0x0020,
		DB6  = 0x0040,
		DB7  = 0x0080,
		WR   = 0x0100,
		RD   = 0x0200,
		CE   = 0x0400,
		CD   = 0x0800,
		RST  = 0x1000,	
		BL   = 0x2000,
		ALL  = 0x3FFF,
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
	T6963C(IGPIO& io, uint8_t rows, uint8_t columns);
	void WriteRow(uint32_t y, uint8_t* data, size_t size);
	uint8_t GetRows();
	uint8_t GetColumns();
};

DEFINE_ENUM_CLASS_FLAG_OPERATORS(T6963C::Pins, uint32_t);
