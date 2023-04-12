#include "t6963c.h"
#include "freertos/task.h"
#include "esp_log.h"


//https://github.com/camilstaps/T6963C_PIC/blob/master/t6963c.c

#define	GLCD_PIN_NONE		MCP23S17_PIN_NONE
#define	GLCD_PIN_WR			MCP23S17_PIN_B0
#define	GLCD_PIN_RD			MCP23S17_PIN_B1
#define	GLCD_PIN_CD			MCP23S17_PIN_B2
#define	GLCD_PIN_CE			MCP23S17_PIN_NONE
#define	GLCD_PIN_BL1		MCP23S17_PIN_B3
#define	GLCD_PIN_BL2		MCP23S17_PIN_B4
#define	GLCD_PIN_DB0		MCP23S17_PIN_A0
#define	GLCD_PIN_DB1		MCP23S17_PIN_A1
#define	GLCD_PIN_DB2		MCP23S17_PIN_A2
#define	GLCD_PIN_DB3		MCP23S17_PIN_A3
#define	GLCD_PIN_DB4		MCP23S17_PIN_A4
#define	GLCD_PIN_DB5		MCP23S17_PIN_A5
#define	GLCD_PIN_DB6		MCP23S17_PIN_A6
#define	GLCD_PIN_DB7		MCP23S17_PIN_A7

#define GLCD_CTRL_PINS		(GLCD_PIN_WR | GLCD_PIN_RD | GLCD_PIN_CD | GLCD_PIN_CE)
#define GLCD_DATA_PINS		(GLCD_PIN_DB0 | GLCD_PIN_DB1 | GLCD_PIN_DB2 | GLCD_PIN_DB3 | GLCD_PIN_DB4 | GLCD_PIN_DB5 | GLCD_PIN_DB6 | GLCD_PIN_DB7 )
#define GLCD_ALL_PINS		(GLCD_CTRL_PINS | GLCD_DATA_PINS | GLCD_PIN_BL1 | GLCD_PIN_BL2)



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


bool T6963C::Init(MCP23S17* expander)
{
	this->expander = expander;
	this->columns = settings.width / 8;
	this->rows = settings.height;
	expander->SetPinsMode(GLCD_ALL_PINS, MCP23S17_PINMODE_OUTPUT);
	expander->SetPins(GLCD_CTRL_PINS, GLCD_CTRL_PINS);
	//RESET, NOT POSSIBLE
	
	SetBacklight(T6963C_BACKLIGHT_MIN);
	
	WriteCmd(T6963_SET_GRAPHIC_HOME_ADDRESS, 0x00, 0x00);
	WriteCmd(T6963_SET_GRAPHIC_AREA, columns, 0x00);			
	WriteCmd(T6963_SET_TEXT_HOME_ADDRESS, 0x00, 0x03);             
	WriteCmd(T6963_SET_TEXT_AREA, columns, 0x00);			
	WriteCmd(T6963_SET_OFFSET_REGISTER, 0b11110, 0x00);		
	
	WriteCmd(T6963_SET_OFFSET_REGISTER);
	WriteCmd(T6963_DISPLAY_MODE | 0b1000);
	WriteCmd(T6963_MODE_SET);
	
	Clear();
	return true;
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
	mcp23s17_pins_t pins = MCP23S17_PIN_NONE;
	if (wr) pins |= GLCD_PIN_WR;
	if (rd) pins |= GLCD_PIN_RD;
	if (cd) pins |= GLCD_PIN_CD;
	if (ce) pins |= GLCD_PIN_CE;
	expander->SetPins(GLCD_CTRL_PINS, pins);
}


void T6963C::SetDataPins(uint8_t data)
{
	expander->SetPins(GLCD_DATA_PINS, (mcp23s17_pins_t)data);
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

void T6963C::SetBacklight(t6963c_backlight_t value)
{
	
	switch (value)
	{
	default:
	case T6963C_BACKLIGHT_OFF:
		expander->SetPins(GLCD_PIN_BL1 | GLCD_PIN_BL2, GLCD_PIN_NONE);
		break;
	case T6963C_BACKLIGHT_MIN:
		expander->SetPins(GLCD_PIN_BL1 | GLCD_PIN_BL2, GLCD_PIN_BL1);
		break;
	case T6963C_BACKLIGHT_MID:
		expander->SetPins(GLCD_PIN_BL1 | GLCD_PIN_BL2, GLCD_PIN_BL2);
		break;
	case T6963C_BACKLIGHT_MAX:
		expander->SetPins(GLCD_PIN_BL1 | GLCD_PIN_BL2, GLCD_PIN_BL1 | GLCD_PIN_BL2);
		break;
	}
}

//--------------------------------------------------------------//
void T6963C::OSet(uint8_t data, bool wr, bool rd, bool cd, bool ce, mcp23s17_pins_t* order)
{
	*order = GLCD_PIN_NONE;
	if (wr) (*order) |= GLCD_PIN_WR;
	if (rd) (*order) |= GLCD_PIN_RD;
	if (cd) (*order) |= GLCD_PIN_CD;
	if (ce) (*order) |= GLCD_PIN_CE;
	(*order) |= ((mcp23s17_pins_t)data) & GLCD_DATA_PINS;
}



mcp23s17_pins_t* T6963C::OWriteByte(bool cd, uint8_t data, mcp23s17_pins_t* order)
{
	OSet(data, 0, 1, cd, 0, order++);
	OSet(data, 1, 1, cd, 1, order++);
	return order;
}

mcp23s17_pins_t* T6963C::OWriteCmd(uint8_t cmd, uint8_t data1, mcp23s17_pins_t* order)
{
	order = OWriteByte(0, data1, order);
	order = OWriteByte(1, cmd, order);
	return order;
}


mcp23s17_pins_t* T6963C::OWriteCmd(uint8_t cmd, uint8_t data1, uint8_t data2, mcp23s17_pins_t* order)
{
	order = OWriteByte(0, data1, order);
	order = OWriteByte(0, data2, order);
	order = OWriteByte(1, cmd, order);
	return order;
}

mcp23s17_pins_t* T6963C::OWriteCmd(uint8_t cmd, mcp23s17_pins_t* order)
{
	order = OWriteByte(1, cmd, order);
	return order;
}

mcp23s17_pins_t* T6963C::OSetAddress(uint8_t col, uint8_t row, mcp23s17_pins_t* order)
{
	uint16_t address = ADDR(col, row);
	order = OWriteCmd(0x24, address, (address >> 8), order);
	return order;
}

void T6963C::WriteRow(uint32_t y, uint8_t* data, size_t size)
{
	mcp23s17_pins_t pinOrder[70];	//6 SetAddr + 2 StartAW + 60 AW + 2 StopAW
	mcp23s17_pins_t* wrPtr = pinOrder;
	wrPtr = OSetAddress(0, y, wrPtr);
	wrPtr = OWriteCmd(T6963_SET_DATA_AUTO_WRITE, wrPtr);	
	int col;
	for (col = 0; col < size; col++)
		wrPtr = OWriteByte(0, data[col], wrPtr);	
	wrPtr = OWriteCmd(T6963_AUTO_RESET, wrPtr);	
	expander->ConsecutivePinWriting(GLCD_CTRL_PINS | GLCD_DATA_PINS, pinOrder, 70);
}
