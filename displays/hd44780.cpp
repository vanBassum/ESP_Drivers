#include "hd44780.h"


//#define LCD_DATA_PINS	(Pins::DB0 | Pins::DB1 | Pins::DB2 | Pins::DB3 | Pins::DB4 | Pins::DB5 | Pins::DB6 | Pins::DB7)
//#define LCD_ALL_PINS	(Pins::RS | Pins::RW | Pins::E | LCD_DATA_PINS | Pins::BL)

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

Result HD44780::DeviceSetConfig(IDeviceConfig &config)
{
	const char* temp;
    ContextLock lock(mutex);
	RETURN_ON_ERR(config.getProperty("gpioDevice", &gpioDeviceKey));

	RETURN_ON_ERR(config.getProperty("pinD0", &temp));
	if(sscanf(temp, "%hhu,%hhu", &d0_Port, &d0_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinD1", &temp));
	if(sscanf(temp, "%hhu,%hhu", &d1_Port, &d1_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinD2", &temp));
	if(sscanf(temp, "%hhu,%hhu", &d2_Port, &d2_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinD3", &temp));
	if(sscanf(temp, "%hhu,%hhu", &d3_Port, &d3_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinD4", &temp));
	if(sscanf(temp, "%hhu,%hhu", &d4_Port, &d4_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinD5", &temp));
	if(sscanf(temp, "%hhu,%hhu", &d5_Port, &d5_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinD6", &temp));
	if(sscanf(temp, "%hhu,%hhu", &d6_Port, &d6_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinD7", &temp));
	if(sscanf(temp, "%hhu,%hhu", &d7_Port, &d7_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinE", &temp));
	if(sscanf(temp, "%hhu,%hhu", &e_Port, &e_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinRS", &temp));
	if(sscanf(temp, "%hhu,%hhu", &rs_Port, &rs_Pin) != 2) return Result::Error;
	RETURN_ON_ERR(config.getProperty("pinBL", &temp));
	if(sscanf(temp, "%hhu,%hhu", &bl_Port, &bl_Pin) != 2) return Result::Error;

	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result HD44780::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(deviceManager->getDeviceByKey<IGpio>(gpioDeviceKey, gpioDevice));

	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

HD44780::Pins FromData(uint8_t data)
{
	//return (HD44780::Pins)data;
}

Result HD44780::DeviceInit()
{
	ContextLock lock(mutex);
    //RETURN_ON_ERR(rwPin.DeviceInit());
    RETURN_ON_ERR(rs_Pin.DeviceInit());
    RETURN_ON_ERR(e_Pin.DeviceInit());
	RETURN_ON_ERR(bl_Pin.DeviceInit());

 const GpioConfig outputConfig = GPIO_CREATE_CONFIG(GpioConfigMode::GPIO_CFG_MODE_OUTPUT, GpioConfigIntr::GPIO_CFG_INTR_DISABLE, GpioConfigPull::GPIO_CFG_PULL_DISABLE);
	//Initialize non-SPI GPIOs
    //RETURN_ON_ERR(rwPin.GpioConfigure(&gpioOutput));
    RETURN_ON_ERR(rs_Pin.GpioConfigure(&outputConfig));
    RETURN_ON_ERR(e_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(bl_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(d0_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(d1_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(d2_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(d3_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(d4_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(d5_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(d6_Pin.GpioConfigure(&outputConfig));
	RETURN_ON_ERR(d7_Pin.GpioConfigure(&outputConfig));

	//RETURN_ON_ERR(rwPin.GpioPinWrite(0));
	RETURN_ON_ERR(rs_Pin.GpioPinWrite(0));
	RETURN_ON_ERR(e_Pin.GpioPinWrite(0));
	RETURN_ON_ERR(bl_Pin.GpioPinWrite(0));
	
	RETURN_ON_ERR(ALL.GpioPinWrite(0));		//All pins low

    //Turn on backlight
    RETURN_ON_ERR(bl_Pin.GpioPinWrite(1));   

	ESP_LOGI(TAG, "Initializing");

	vTaskDelay(pdMS_TO_TICKS(100));								//Wait for screen

	vTaskDelay(pdMS_TO_TICKS(10));	
	//io.SetPins(LCD_DATA_PINS, FromData(0x30) | Pins::E);
	RETURN_ON_ERR(rs_Pin.GpioPinWrite(0));	
	RETURN_ON_ERR(e_Pin.GpioPinWrite(0));	
	
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
		//io.SetPins(Pins::BL, Pins::BL);
		RETURN_ON_ERR(bl_Pin.GpioPinWrite(1));
	else		
		//io.SetPins(Pins::BL, Pins::NONE);
		RETURN_ON_ERR(bl_Pin.GpioPinWrite(0));
	
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
	//io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::E);
	RETURN_ON_ERR(e_Pin.GpioPinWrite(1));
	vTaskDelay(pdMS_TO_TICKS(10));	
	//io.SetPins(LCD_DATA_PINS, FromData(cmd));
	RETURN_ON_ERR(data_Pin.GpioPinWrite(cmd));
	vTaskDelay(pdMS_TO_TICKS(10));	
	//io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::NONE);
	RETURN_ON_ERR(e_Pin.GpioPinWrite(0));
}


void HD44780::WaitBFClear()
{
	vTaskDelay(pdMS_TO_TICKS(1));
	//ets_delay_us(100);	//TODO
}


void HD44780::LCD_Data(unsigned char cmd)
{
	//io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::RS | Pins::E);
	RETURN_ON_ERR(rs_Pin.GpioPinWrite(1));
	RETURN_ON_ERR(e_Pin.GpioPinWrite(1));
	//io.SetPins(LCD_DATA_PINS, FromData(cmd));
	RETURN_ON_ERR(data_Pin.GpioPinWrite(cmd));
	vTaskDelay(pdMS_TO_TICKS(1));	//TODO Whaaaa?? This HAS to be �S...
	//ets_delay_us(1);
	//io.SetPins(Pins::E | Pins::RW | Pins::RS, Pins::NONE);
	RETURN_ON_ERR(rs_Pin.GpioPinWrite(0));
	RETURN_ON_ERR(e_Pin.GpioPinWrite(0));	
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
