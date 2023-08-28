#include "t6963c.h"
#include "freertos/task.h"
#include "esp_log.h"


//https://github.com/camilstaps/T6963C_PIC/blob/master/t6963c.c
#define GLCD_CTRL_PINS		(Pins::WR | Pins::RD | Pins::CD | Pins::CE)
#define GLCD_DATA_PINS		(Pins::DB0 | Pins::DB1 | Pins::DB2 | Pins::DB3 | Pins::DB4 | Pins::DB5 | Pins::DB6 | Pins::DB7 )

#define T6963_CURSOR_PATTERN_SELECT 		0xA0
#define T6963_DISPLAY_MODE 				    0x90
#define T6963_MODE_SET 					    0x80
#define T6963_SET_CURSOR_POINTER 		    0x21
#define T6963_SET_OFFSET_REGISTER 		    0x22
#define T6963_SET_ADDRESS_POINTER 		    0x24
#define T6963_SET_TEXT_HOME_ADDRESS 	    0x40
#define T6963_SET_TEXT_AREA 			    0x41
#define T6963_SET_GRAPHIC_HOME_ADDRESS 	    0x42
#define T6963_SET_GRAPHIC_AREA 			    0x43
#define T6963_SET_DATA_AUTO_WRITE 		    0xB0
#define T6963_SET_DATA_AUTO_READ 		    0xB1
#define T6963_AUTO_RESET 				    0xB2
                                            
#define T6963_DATA_WRITE_AND_INCREMENT 	    0xC0
#define T6963_DATA_READ_AND_INCREMENT 	    0xC1
#define T6963_DATA_WRITE_AND_DECREMENT 	    0xC2
#define T6963_DATA_READ_AND_DECREMENT 	    0xC3
#define T6963_DATA_WRITE_AND_NONVARIALBE    0xC4
#define T6963_DATA_READ_AND_NONVARIABLE     0xC5
                                            
#define T6963_SCREEN_PEEK 				    0xE0
#define T6963_SCREEN_COPY 				    0xE8

#define ADDR(col, row)	(col + (row * 30))

T6963C::T6963C(IGPIO& io, uint8_t rows, uint8_t columns)
	: io(io)
{
	ESP_LOGI(TAG, "Initializing");
	this->columns = columns;
	this->rows = rows;
	
	io.SetPinsMode(Pins::ALL,	PinModes::PIN_OUTPUT);
	io.SetPins(Pins::ALL,		Pins::NONE);
	io.SetPins(Pins::RST,		Pins::RST);	

	WriteCmd(T6963_SET_GRAPHIC_HOME_ADDRESS, 0x00, 0x00);
	WriteCmd(T6963_SET_GRAPHIC_AREA, columns, 0x00);			
	WriteCmd(T6963_SET_TEXT_HOME_ADDRESS, 0x00, 0x03);             
	WriteCmd(T6963_SET_TEXT_AREA, columns, 0x00);			
	WriteCmd(T6963_SET_OFFSET_REGISTER, 0b11110, 0x00);		
	
	WriteCmd(T6963_SET_OFFSET_REGISTER);
	WriteCmd(T6963_DISPLAY_MODE | 0b1000);
	WriteCmd(T6963_MODE_SET);
	
	Clear();
}

void T6963C::WriteCmd(uint8_t cmd, uint8_t data1)
{
	StatusCheck();
	WriteByte(0, data1);
	StatusCheck();
	WriteByte(1, cmd);
}


void T6963C::WriteCmd(uint8_t cmd, uint8_t data1, uint8_t data2)
{
	StatusCheck();
	WriteByte(0, data1);
	StatusCheck();
	WriteByte(0, data2);
	StatusCheck();
	WriteByte(1, cmd);
}

void T6963C::WriteCmd(uint8_t cmd)
{
	StatusCheck();
	WriteByte(1, cmd);
}


void T6963C::WriteByte(bool cd, uint8_t data)
{
	SetDataPins(data);
	SetCtrlPins(0, 1, cd, 0);
	//200 ns delay
	SetCtrlPins(1, 1, cd, 1);
	//200 ns delay
}


void T6963C::SetCtrlPins(bool wr, bool rd, bool cd, bool ce)
{
	Pins pins = Pins::NONE;
	if (wr) pins |= Pins::WR;
	if (rd) pins |= Pins::RD;
	if (cd) pins |= Pins::CD;
	if (ce) pins |= Pins::CE;
	io.SetPins(GLCD_CTRL_PINS, pins);
}


void T6963C::SetDataPins(uint8_t data)
{
	io.SetPins(GLCD_DATA_PINS, (Pins)data);
}



void T6963C::SetAddress(uint8_t col, uint8_t row)
{
	uint16_t address = ADDR(col, row);
	WriteCmd(0x24, address & 0xff, ((address >> 8) & 0xff));
}


void T6963C::SetCursor(uint8_t col, uint8_t row)
{
	WriteCmd(0x21, col, row);
}


void T6963C::Clear()
{
	uint16_t i;
	SetAddress(0, 0);
	StartAutoWrite();
	for (i = 0; i < rows * columns; i++)
	{
		AutoWrite(0x00);
	}
	WriteCmd(0x24, 0x00, 0x03);
	for (i = 0; i < rows * columns; i++)
	{
		AutoWrite(0x00);	//t6963c_attr_normal 
	}
	StopAutoWrite();
	SetCursor(0, 0);
}


void T6963C::StartAutoWrite()
{
	WriteCmd(0xb0);
	//delay 120us
}


void T6963C::StopAutoWrite()
{
	WriteCmd(0xb2);
	//delay 120us
}


void T6963C::AutoWrite(uint8_t data)
{
	SetDataPins(data);
	SetCtrlPins(0, 1, 0, 0);
	//delay 200ns
	SetCtrlPins(1, 1, 1, 1);
}


void T6963C::AutoWriteChar(char data)
{
	AutoWrite(data - 0x20);
}


//--------------------------------------------------------------//
void T6963C::OSet(uint8_t data, bool wr, bool rd, bool cd, bool ce, Pins* order)
{
	*order = Pins::NONE;
	if (wr) (*order) |= Pins::WR;
	if (rd) (*order) |= Pins::RD;
	if (cd) (*order) |= Pins::CD;
	if (ce) (*order) |= Pins::CE;
	(*order) |= ((Pins)data) & GLCD_DATA_PINS;
}



T6963C::Pins* T6963C::OWriteByte(bool cd, uint8_t data, Pins* order)
{
	OSet(data, 0, 1, cd, 0, order++);
	OSet(data, 1, 1, cd, 1, order++);
	return order;
}

T6963C::Pins* T6963C::OWriteCmd(uint8_t cmd, uint8_t data1, Pins* order)
{
	order = OWriteByte(0, data1, order);
	order = OWriteByte(1, cmd, order);
	return order;
}


T6963C::Pins* T6963C::OWriteCmd(uint8_t cmd, uint8_t data1, uint8_t data2, Pins* order)
{
	order = OWriteByte(0, data1, order);
	order = OWriteByte(0, data2, order);
	order = OWriteByte(1, cmd, order);
	return order;
}

T6963C::Pins* T6963C::OWriteCmd(uint8_t cmd, Pins* order)
{
	order = OWriteByte(1, cmd, order);
	return order;
}

T6963C::Pins* T6963C::OSetAddress(uint8_t col, uint8_t row, Pins* order)
{
	uint16_t address = ADDR(col, row);
	order = OWriteCmd(0x24, address, (address >> 8), order);
	return order;
}

void T6963C::WriteRow(uint32_t y, uint8_t* data, size_t size)
{
	Pins pinOrder[70];	//6 SetAddr + 2 StartAW + 60 AW + 2 StopAW
	Pins* wrPtr = pinOrder;
	wrPtr = OSetAddress(0, y, wrPtr);
	wrPtr = OWriteCmd(T6963_SET_DATA_AUTO_WRITE, wrPtr);	
	int col;
	for (col = 0; col < size; col++)
		wrPtr = OWriteByte(0, data[col], wrPtr);	
	wrPtr = OWriteCmd(T6963_AUTO_RESET, wrPtr);	
	io.ConsecutivePinWriting(GLCD_CTRL_PINS | GLCD_DATA_PINS, pinOrder, 70);
}


uint8_t T6963C::GetColumns()
{
	return columns;
}


uint8_t T6963C::GetRows()
{
	return rows;
}
