#include "hd44780.h"

#define LCD_DATA_PORT	(uint32_t)(0)
#define LCD_DATA_MASK	0xFF
#define LCD_CMD_PORT	(uint32_t)(1)
#define LCD_RS_PIN		(uint8_t)(1)
#define LCD_E_PIN		(uint8_t)(2)
#define LCD_BL_PIN		(uint8_t)(0x18)

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
    ContextLock lock(mutex);
	RETURN_ON_ERR_LOGD(config.getProperty("mcpdevice", &mcpDeviceKey), TAG,  "Setconfig Error");

	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result HD44780::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGD(deviceManager->getDeviceByKey<MCP23S17>(mcpDeviceKey, mcpDevice), TAG , "load device error");

	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result HD44780::DeviceInit()
{
	ContextLock lock(mutex);
	const GpioConfig outputConfig = GPIO_CREATE_CONFIG(GpioConfigMode::GPIO_CFG_MODE_OUTPUT, GpioConfigIntr::GPIO_CFG_INTR_DISABLE, GpioConfigPull::GPIO_CFG_PULL_DISABLE);

	RETURN_ON_ERR_LOGD(mcpDevice->GpioConfigure(LCD_DATA_PORT,LCD_DATA_MASK,&outputConfig),TAG, "GPIOconfig Error1");
	RETURN_ON_ERR_LOGD(mcpDevice->GpioConfigure(LCD_CMD_PORT,LCD_E_PIN,&outputConfig),TAG, "GPIOconfig Error2");
	RETURN_ON_ERR_LOGD(mcpDevice->GpioConfigure(LCD_CMD_PORT,LCD_BL_PIN,&outputConfig),TAG, "GPIOconfig Error3");
	RETURN_ON_ERR_LOGD(mcpDevice->GpioConfigure(LCD_CMD_PORT,LCD_RS_PIN,&outputConfig),TAG, "GPIOconfig Error4");
	
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_DATA_PORT, LCD_DATA_MASK, 0));
	RETURN_ON_ERR_LOGD(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_RS_PIN, 0),TAG, "GPIO write Error1");
	RETURN_ON_ERR_LOGD(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_E_PIN, 0),TAG, "GPIO write Error2" );
	RETURN_ON_ERR_LOGD(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_BL_PIN, LCD_BL_PIN),TAG, "GPIO write Error3");
	
	RETURN_ON_ERR(LCD_cmd(HD44780_INIT_SEQ));

    //Turn on backlight
  	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_BL_PIN, 0)); 

	ESP_LOGI(TAG, "Initializing");

	vTaskDelay(pdMS_TO_TICKS(100));								//Wait for screen

	RETURN_ON_ERR(LCD_cmd(HD44780_INIT_SEQ));
	vTaskDelay(pdMS_TO_TICKS(105)); //+5	 
	//ets_delay_us(4500);

	RETURN_ON_ERR(LCD_cmd(HD44780_INIT_SEQ));
	vTaskDelay(pdMS_TO_TICKS(101));	//+1
	//ets_delay_us(120);

	RETURN_ON_ERR(LCD_cmd(HD44780_INIT_SEQ));
	vTaskDelay(pdMS_TO_TICKS(101));	//+1
	//ets_delay_us(50);

	RETURN_ON_ERR(LCD_cmd(HD44780_8BIT_MODE | HD44780_2_ROWS | HD44780_FONT_5x10));
	RETURN_ON_ERR(LCD_cmd(HD44780_DISP_OFF));
	RETURN_ON_ERR(LCD_cmd(HD44780_DISP_CLEAR));
	RETURN_ON_ERR(LCD_cmd(HD44780_ENTRY_MODE | HD44780_RETURN_HOME));
	RETURN_ON_ERR(LCD_cmd(HD44780_DISP_ON));
	RETURN_ON_ERR(LCD_cmd(HD44780_CURSOR_OFF));
	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
	//WaitBFClear();
}

Result HD44780::SetBacklight(bool enabled)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));
	if (enabled)
  		RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_BL_PIN, 0)); 
	else		
  		RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_BL_PIN, LCD_BL_PIN)); 	
	return Result::Ok;
}



void HD44780::SetCursor(int x, int row)
{
	x %= 16;
	row %= 2;
	if (row == 1) x += 64;
	LCD_cmd(HD44780_POSITION + x);
}

Result HD44780::LCD_cmd(unsigned char cmd)
{
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_E_PIN, LCD_E_PIN));
	vTaskDelay(pdMS_TO_TICKS(10));	
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_DATA_PORT, LCD_DATA_MASK, cmd));
	vTaskDelay(pdMS_TO_TICKS(10));	
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_E_PIN, 0));
	return Result::Ok;
}

void HD44780::WaitBFClear()
{
	vTaskDelay(pdMS_TO_TICKS(1));
	//ets_delay_us(100);	//TODO
}

Result HD44780::LCD_Data(unsigned char cmd)
{
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_RS_PIN, LCD_RS_PIN));
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_E_PIN, LCD_E_PIN));
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_DATA_PORT, LCD_DATA_MASK, cmd));
	vTaskDelay(pdMS_TO_TICKS(1));	//TODO Whaaaa?? This HAS to be �S...
	//ets_delay_us(1);
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_E_PIN, 0));
	RETURN_ON_ERR(mcpDevice->GpioWrite(LCD_CMD_PORT, LCD_RS_PIN, 0));
	return Result::Ok;
	WaitBFClear();
}

Result HD44780::printf(const char *format, ...)
{
    ContextLock lock(mutex);
    RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));

    va_list args;
    va_start(args, format);

    char buffer[256]; // Adjust the buffer size as per your requirements

    // Print formatted message to buffer
    int written = vsnprintf(buffer, sizeof(buffer), format, args);
    if (written < 0 || written >= sizeof(buffer)) {
        // Handle error if vsnprintf fails or the buffer is not large enough
        va_end(args);
        return Result::Error;
    }

    va_end(args);

    // Write each character from the buffer to the LCD
    for (int i = 0; i < written; i++) {
        RETURN_ON_ERR(LCD_Data(buffer[i]));
    }

    return Result::Ok;
}

Result HD44780::printf(int x, int y, const char *format, ...)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));
	SetCursor(x, y);
    va_list args;
    va_start(args, format);

    char buffer[256]; // Adjust the buffer size as per your requirements

    // Print formatted message to buffer
    int written = vsnprintf(buffer, sizeof(buffer), format, args);
    if (written < 0 || written >= sizeof(buffer)) {
        // Handle error if vsnprintf fails or the buffer is not large enough
        va_end(args);
        return Result::Error;
    }

    va_end(args);

    // Write each character from the buffer to the LCD
    for (int i = 0; i < written; i++) {
        RETURN_ON_ERR(LCD_Data(buffer[i]));
    }

    return Result::Ok;
}
