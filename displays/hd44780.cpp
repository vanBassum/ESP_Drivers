#include "hd44780.h"
#include "kernel.h"

#define LCD_DATA_PINS	(Pins::DB0 | Pins::DB1 | Pins::DB2 | Pins::DB3 | Pins::DB4 | Pins::DB5 | Pins::DB6 | Pins::DB7)
#define LCD_ALL_PINS	(Pins::RS | Pins::RW | Pins::E | LCD_DATA_PINS | Pins::BL)

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

HD44780::Pins FromData(uint8_t data)
{
	return (HD44780::Pins)data;
}

HD44780::HD44780(IGPIO& io)
	: io(io)
{
	ESP_LOGI(TAG, "Initializing");
	io.SetPinsMode(LCD_ALL_PINS, PinModes::PIN_OUTPUT);
	io.SetPins(LCD_ALL_PINS, Pins::NONE);				//All pins low
	vTaskDelay(pdMS_TO_TICKS(100));								//Wait for screen

	io.SetPins(LCD_DATA_PINS, FromData(0xFF));
	vTaskDelay(pdMS_TO_TICKS(10));	
	
	io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::E);
	vTaskDelay(pdMS_TO_TICKS(10));	
	
	io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::NONE);
	vTaskDelay(pdMS_TO_TICKS(10));
	
	
	
	
	vTaskDelay(pdMS_TO_TICKS(1000));	
	
	
	
	
	vTaskDelay(pdMS_TO_TICKS(10));	
	
	vTaskDelay(pdMS_TO_TICKS(10));	
	io.SetPins(Pins::E | Pins::RW | Pins::RS, FromData(0x30));
	vTaskDelay(pdMS_TO_TICKS(10));	
	io.SetPins(LCD_DATA_PINS, FromData(0x30) | Pins::E);
	
	
	
	
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
}





void HD44780::SetBacklight(bool enabled)
{
	if (enabled)
		io.SetPins(Pins::BL, Pins::BL);
	else		
		io.SetPins(Pins::BL, Pins::NONE);
	
}


void HD44780::SetCursor(int x, int row)
{
	x %= 16;
	row %= 2;
	if (row == 1) x += 64;
	LCD_cmd(HD44780_POSITION + x);
}


void HD44780::LCD_cmd(unsigned char cmd)
{
	io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::E);
	vTaskDelay(pdMS_TO_TICKS(10));	
	io.SetPins(LCD_DATA_PINS, FromData(cmd));
	vTaskDelay(pdMS_TO_TICKS(10));	
	io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::NONE);
}


void HD44780::WaitBFClear()
{
	vTaskDelay(pdMS_TO_TICKS(1));
	//ets_delay_us(100);	//TODO
}


void HD44780::LCD_Data(unsigned char cmd)
{
	io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::RS | Pins::E);
	io.SetPins(LCD_DATA_PINS, FromData(cmd));
	vTaskDelay(pdMS_TO_TICKS(1));	//TODO Whaaaa?? This HAS to be µS...
	//ets_delay_us(1);
	io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::NONE);
	WaitBFClear();
}


void HD44780::Write(std::string message, int x, int y)
{
	SetCursor(x, y);
	Write(message);
}


void HD44780::Write(std::string message)
{
	for (int i = 0; i < message.length(); i++)
	{
		LCD_Data(message.c_str()[i]);
	}
}
