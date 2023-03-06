#include "hd44780.h"
#include "rtos/task.h"

#define	LCD_PIN_NONE	MCP23S17_PIN_NONE
#define	LCD_PIN_RW		MCP23S17_PIN_NONE
#define	LCD_PIN_RS		MCP23S17_PIN_B0
#define	LCD_PIN_E		MCP23S17_PIN_B1
#define	LCD_PIN_BL		MCP23S17_PIN_B4
#define	LCD_PIN_DB0		MCP23S17_PIN_A0
#define	LCD_PIN_DB1		MCP23S17_PIN_A1
#define	LCD_PIN_DB2		MCP23S17_PIN_A2
#define	LCD_PIN_DB3		MCP23S17_PIN_A3
#define	LCD_PIN_DB4		MCP23S17_PIN_A4
#define	LCD_PIN_DB5		MCP23S17_PIN_A5
#define	LCD_PIN_DB6		MCP23S17_PIN_A6
#define	LCD_PIN_DB7		MCP23S17_PIN_A7


#define LCD_DATA_PINS	(LCD_PIN_DB0 | LCD_PIN_DB1 | LCD_PIN_DB2 | LCD_PIN_DB3 | LCD_PIN_DB4 | LCD_PIN_DB5 | LCD_PIN_DB6 | LCD_PIN_DB7)
#define LCD_ALL_PINS	(LCD_PIN_RS | LCD_PIN_RW | LCD_PIN_E | LCD_DATA_PINS | LCD_PIN_BL)

#define HD44780_INIT_SEQ        0x30
#define HD44780_DISP_CLEAR      0x01
#define HD44780_DISP_OFF        0x08
#define HD44780_DISP_ON         0x0C
#define HD44780_CURSOR_ON       0x0E
#define HD44780_CURSOR_OFF      0x0C
#define HD44780_CURSOR_BLINK    0x0F
#define HD44780_RETURN_HOME     0x02 
#define HD44780_ENTRY_MODE      0x06
#define HD44780_4BIT_MODE       0x20
#define HD44780_8BIT_MODE       0x30
#define HD44780_2_ROWS          0x08
#define HD44780_FONT_5x8        0x00
#define HD44780_FONT_5x10       0x04
#define HD44780_POSITION        0x80

#define HD44780_SHIFT           0x10
#define HD44780_CURSOR          0x00
#define HD44780_DISPLAY         0x08
#define HD44780_LEFT            0x00
#define HD44780_RIGHT           0x04

#define HD44780_ROWS            2
#define HD44780_COLS            16

#define HD44780_ROW1_START      0x00
#define HD44780_ROW1_END        HD44780_COLS
#define HD44780_ROW2_START      0x40
#define HD44780_ROW2_END        HD44780_COLS

ESP_Drivers::mcp23s17_pins_t FromData(uint8_t data)
{
	return (ESP_Drivers::mcp23s17_pins_t)data;
}


esp_err_t ESP_Drivers::HD44780::Init(MCP23S17* expander)
{
	this->expander = expander;
	expander->SetPinsMode(LCD_ALL_PINS, MCP23S17_PINMODE_OUTPUT);
	expander->SetPins(LCD_ALL_PINS, LCD_PIN_NONE);				//All pins low
	vTaskDelay(pdMS_TO_TICKS(100));								//Wait for screen
	
	
	
	expander->SetPins(LCD_DATA_PINS, FromData(0xFF));
	vTaskDelay(pdMS_TO_TICKS(10));	
	
	expander->SetPins(LCD_PIN_E | LCD_PIN_RW | LCD_PIN_RS, LCD_PIN_E);
	vTaskDelay(pdMS_TO_TICKS(10));	
	
	expander->SetPins(LCD_PIN_E | LCD_PIN_RW | LCD_PIN_RS, LCD_PIN_NONE);
	vTaskDelay(pdMS_TO_TICKS(10));
	
	
	
	
	vTaskDelay(pdMS_TO_TICKS(1000));	
	
	
	
	
	vTaskDelay(pdMS_TO_TICKS(10));	
	
	vTaskDelay(pdMS_TO_TICKS(10));	
	expander->SetPins(LCD_PIN_E | LCD_PIN_RW | LCD_PIN_RS, FromData(0x30));
	vTaskDelay(pdMS_TO_TICKS(10));	
	expander->SetPins(LCD_DATA_PINS, FromData(0x30) | LCD_PIN_E);
	
	
	
	
	LCD_cmd(HD44780_INIT_SEQ);
	vTaskDelay(pdMS_TO_TICKS(105)); //+5	 
	//ets_delay_us(4500);
	
	LCD_cmd(HD44780_INIT_SEQ);
	vTaskDelay(pdMS_TO_TICKS(101));	//+1
	//ets_delay_us(120);
	
	LCD_cmd(HD44780_INIT_SEQ);
	vTaskDelay(pdMS_TO_TICKS(101));	//+1
	//ets_delay_us(50);
	
	LCD_cmd(HD44780_8BIT_MODE | HD44780_2_ROWS | HD44780_FONT_5x10);
	LCD_cmd(HD44780_DISP_OFF);
	LCD_cmd(HD44780_DISP_CLEAR);
	LCD_cmd(HD44780_ENTRY_MODE | HD44780_RETURN_HOME);
	LCD_cmd(HD44780_DISP_ON);
	LCD_cmd(HD44780_CURSOR_OFF);
	
	
	
	//WaitBFClear();
	
	
	return ESP_OK;
}





void ESP_Drivers::HD44780::SetBacklight(bool enabled)
{
	if (enabled)
		expander->SetPins(LCD_PIN_BL, LCD_PIN_BL);
	else		
		expander->SetPins(LCD_PIN_BL, LCD_PIN_NONE);
	
}


void ESP_Drivers::HD44780::SetCursor(int x, int row)
{
	x %= 16;
	row %= 2;
	if (row == 1) x += 64;
	LCD_cmd(HD44780_POSITION + x);
}


void ESP_Drivers::HD44780::LCD_cmd(unsigned char cmd)
{
	expander->SetPins(LCD_PIN_E | LCD_PIN_RW | LCD_PIN_RS, LCD_PIN_E);
	vTaskDelay(pdMS_TO_TICKS(10));	
	expander->SetPins(LCD_DATA_PINS, FromData(cmd));
	vTaskDelay(pdMS_TO_TICKS(10));	
	expander->SetPins(LCD_PIN_E | LCD_PIN_RW | LCD_PIN_RS, LCD_PIN_NONE);
}


void ESP_Drivers::HD44780::WaitBFClear()
{
	vTaskDelay(pdMS_TO_TICKS(1));
	//ets_delay_us(100);	//TODO
}


void ESP_Drivers::HD44780::LCD_Data(unsigned char cmd)
{
	expander->SetPins(LCD_PIN_E | LCD_PIN_RW | LCD_PIN_RS, LCD_PIN_RS | LCD_PIN_E);
	expander->SetPins(LCD_DATA_PINS, FromData(cmd));
	vTaskDelay(pdMS_TO_TICKS(1));	//TODO Whaaaa?? This HAS to be µS...
	//ets_delay_us(1);
	expander->SetPins(LCD_PIN_E | LCD_PIN_RW | LCD_PIN_RS, LCD_PIN_NONE);
	WaitBFClear();
}


void ESP_Drivers::HD44780::Write(std::string message, int x, int y)
{
	SetCursor(x, y);
	Write(message);
}


void ESP_Drivers::HD44780::Write(std::string message)
{
	for (int i = 0; i < message.length(); i++)
	{
		LCD_Data(message.c_str()[i]);
	}
}
