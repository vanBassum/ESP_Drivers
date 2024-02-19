#include "ST7796S.h"


/*******************
 * ST7796S REGS
*********************/

/* MIPI DCS Type1  */
#define ST7796S_CMD_NOP 0x00
#define ST7796S_CMD_SOFTWARE_RESET 0x01
#define ST7796S_CMD_READ_DISP_POWER_MODE 0x0A
#define ST7796S_CMD_READ_DISP_MADCTRL 0x0B // bits 7:3 only
#define ST7796S_CMD_READ_DISP_PIXEL_FORMAT 0x0C
#define ST7796S_CMD_READ_DISP_IMAGE_MODE 0x0D
#define ST7796S_CMD_READ_DISP_SIGNAL_MODE 0x0E
#define ST7796S_CMD_READ_DISP_SELF_DIAGNOSTIC 0x0F // bits 7:6 only
#define ST7796S_CMD_ENTER_SLEEP_MODE 0x10
#define ST7796S_CMD_SLEEP_OUT 0x11
#define ST7796S_CMD_PARTIAL_MODE_ON 0x12
#define ST7796S_CMD_NORMAL_DISP_MODE_ON 0x13
#define ST7796S_CMD_DISP_INVERSION_OFF 0x20
#define ST7796S_CMD_DISP_INVERSION_ON 0x21
#define ST7796S_CMD_DISPLAY_OFF 0x28
#define ST7796S_CMD_DISPLAY_ON 0x29
#define ST7796S_CMD_COLUMN_ADDRESS_SET 0x2A
#define ST7796S_CMD_PAGE_ADDRESS_SET 0x2B
#define ST7796S_CMD_MEMORY_WRITE 0x2C
#define ST7796S_CMD_MEMORY_READ 0x2E
#define ST7796S_CMD_PARTIAL_AREA 0x30
#define ST7796S_CMD_VERT_SCROLL_DEFINITION 0x33
#define ST7796S_CMD_TEARING_EFFECT_LINE_OFF 0x34
#define ST7796S_CMD_TEARING_EFFECT_LINE_ON 0x35
#define ST7796S_CMD_MEMORY_ACCESS_CONTROL 0x36 // bits 7:3,1:0 only
#define ST7796S_CMD_VERT_SCROLL_START_ADDRESS 0x37
#define ST7796S_CMD_IDLE_MODE_OFF 0x38
#define ST7796S_CMD_IDLE_MODE_ON 0x39
#define ST7796S_CMD_COLMOD_PIXEL_FORMAT_SET 0x3A
#define ST7796S_CMD_WRITE_MEMORY_CONTINUE 0x3C
#define ST7796S_CMD_READ_MEMORY_CONTINUE 0x3E
#define ST7796S_CMD_SET_TEAR_SCANLINE 0x44
#define ST7796S_CMD_GET_SCANLINE 0x45

#define ST7796S_DDB_START 0xA1
#define ST7796S_DDB_CONTINUE 0xA8

/* other */
#define ST7796S_CMD_ACCESS_PROTECT 0xB0
#define ST7796S_CMD_LOW_POWER_CONTROL 0xB1
#define ST7796S_CMD_FRAME_MEMORY_ACCESS 0xB3
#define ST7796S_CMD_DISPLAY_MODE 0xB4
#define ST7796S_CMD_DEVICE_CODE 0xBF

#define ST7796S_CMD_PANEL_DRIVE 0xC0
#define ST7796S_CMD_DISP_TIMING_NORMAL 0xC1
#define ST7796S_CMD_DISP_TIMING_PARTIAL 0xC2
#define ST7796S_CMD_DISP_TIMING_IDLE 0xC3
#define ST7796S_CMD_FRAME_RATE 0xC5
#define ST7796S_CMD_INTERFACE_CONTROL 0xC6
#define ST7796S_CMD_GAMMA_SETTING 0xC8

#define ST7796S_CMD_POWER_SETTING 0xD0
#define ST7796S_CMD_VCOM_CONTROL 0xD1
#define ST7796S_CMD_POWER_CONTROL_NORMAL 0xD2
#define ST7796S_CMD_POWER_CONTROL_IDEL 0xD3
#define ST7796S_CMD_POWER_CONTROL_PARTIAL 0xD4

#define ST7796S_CMD_NVMEM_WRITE 0xE0
#define ST7796S_CMD_NVMEM_PROTECTION_KEY 0xE1
#define ST7796S_CMD_NVMEM_STATUS_READ 0xE2
#define ST7796S_CMD_NVMEM_PROTECTION 0xE3


/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct
{
	uint8_t cmd;
	uint8_t data[16];
	uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;


const lcd_init_cmd_t init_cmds[] = {
    {0xCF, {0x00, 0x83, 0X30}, 3},
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    {0xE8, {0x85, 0x01, 0x79}, 3},
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    {0xF7, {0x20}, 1},
    {0xEA, {0x00, 0x00}, 2},
    {0xC0, {0x26}, 1},		 /*Power control*/
    {0xC1, {0x11}, 1},		 /*Power control */
    {0xC5, {0x35, 0x3E}, 2}, /*VCOM control*/
    {0xC7, {0xBE}, 1},		 /*VCOM control*/
    {0x36, {0x28}, 1},		 /*Memory Access Control*/
    {0x3A, {0x55}, 1},		 /*Pixel Format Set*/
    {0xB1, {0x00, 0x1B}, 2},
    {0xF2, {0x08}, 1},
    {0x26, {0x01}, 1},
    {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
    {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
    {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
    {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
    {0x2C, {0}, 0},
    {0xB7, {0x07}, 1},
    {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
    {0x11, {0}, 0x80},
    {0x29, {0}, 0x80},
    {0, {0}, 0xff},
};


const GpioConfig gpioOutput = GPIO_CREATE_CONFIG(GpioMode::GPIO_CFG_MODE_OUTPUT, GpioIntr::GPIO_CFG_INTR_DISABLE, GpioPullFlags::GPIO_CFG_PULL_DISABLE); 

void disp_wait_for_pending_transactions(void)
{
    //TODO: Implement this!
}

DeviceResult ST7796S::st7796s_send_cmd(uint8_t cmd)
{
	disp_wait_for_pending_transactions();
    DEV_RETURN_ON_ERROR_SILENT(dcPin.GpioPinWrite(0));/*Command mode*/
    DEV_RETURN_ON_ERROR_SILENT(st7796s_spi_transfer(&cmd, nullptr, 1));
	return DeviceResult::Ok;
}

DeviceResult ST7796S::st7796s_send_data(const uint8_t * data, uint16_t length)
{
	disp_wait_for_pending_transactions();
    DEV_RETURN_ON_ERROR_SILENT(dcPin.GpioPinWrite(1)); /*Data mode*/
    DEV_RETURN_ON_ERROR_SILENT(st7796s_spi_transfer(data, nullptr, length));
	return DeviceResult::Ok;
}

DeviceResult ST7796S::st7796s_send_color(const uint16_t* data, uint16_t length)
{
	disp_wait_for_pending_transactions();
    DEV_RETURN_ON_ERROR_SILENT(dcPin.GpioPinWrite(1)); /*Data mode*/
    DEV_RETURN_ON_ERROR_SILENT(st7796s_spi_transfer((uint8_t*)data, nullptr, length * 2));
	return DeviceResult::Ok;
}

DeviceResult ST7796S::st7796s_set_orientation(uint8_t orientation)
{
	const char *orientation_str[] = {"PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"};
	uint8_t data[] = {0x48, 0x88, 0x28, 0xE8};
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_cmd(0x36));
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_data(&data[orientation], 1));
	return DeviceResult::Ok;
}

DeviceResult ST7796S::st7796s_spi_transfer(const uint8_t * txData, uint8_t * rxData, size_t length)
{
    if(length == 0)
	{
        ESP_LOGE("TEST", "tx = %p, rx = %p, length = %d", txData, rxData, (int)length);
		return DeviceResult::Ok;
	}
    return spiDevice->SpiTransmit(txData, rxData, length, SPIFlags::POLLED);    //TODO: Use DMA
}



DeviceResult ST7796S::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiDevice", &spiDeviceKey),  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Missing parameter: spiDevice");
    DEV_RETURN_ON_ERROR_SILENT(dcPin.DeviceSetConfig(config, "dcPin"));
    DEV_RETURN_ON_ERROR_SILENT(rstPin.DeviceSetConfig(config, "rstPin"));
    DEV_RETURN_ON_ERROR_SILENT(blckPin.DeviceSetConfig(config, "blckPin"));

	DeviceSetStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult ST7796S::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(spiDevice, deviceManager->getDeviceByKey<ISpiDevice>(spiDeviceKey), DeviceStatus::Dependencies, DeviceResult::Error, TAG, "Dependencies not ready %s", spiDeviceKey);
	DEV_RETURN_ON_ERROR_SILENT(dcPin.DeviceLoadDependencies(deviceManager));
	DEV_RETURN_ON_ERROR_SILENT(rstPin.DeviceLoadDependencies(deviceManager));
	DEV_RETURN_ON_ERROR_SILENT(blckPin.DeviceLoadDependencies(deviceManager));

	DeviceSetStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}

DeviceResult ST7796S::DeviceInit()
{
	ContextLock lock(mutex);
    DEV_RETURN_ON_ERROR_SILENT(dcPin.DeviceInit());
    DEV_RETURN_ON_ERROR_SILENT(rstPin.DeviceInit());
    DEV_RETURN_ON_ERROR_SILENT(blckPin.DeviceInit());


	//Initialize non-SPI GPIOs
    DEV_RETURN_ON_ERROR_SILENT(dcPin.GpioConfigure(&gpioOutput));
    DEV_RETURN_ON_ERROR_SILENT(rstPin.GpioConfigure(&gpioOutput));
    DEV_RETURN_ON_ERROR_SILENT(blckPin.GpioConfigure(&gpioOutput));

    //Default the dc pin
    DEV_RETURN_ON_ERROR_SILENT(dcPin.GpioPinWrite(0));

    //Turn on backlight
    DEV_RETURN_ON_ERROR_SILENT(blckPin.GpioPinWrite(1));   

    //Reset the display
    DEV_RETURN_ON_ERROR_SILENT(rstPin.GpioPinWrite(0));
	vTaskDelay(100 / portTICK_PERIOD_MS);
    DEV_RETURN_ON_ERROR_SILENT(rstPin.GpioPinWrite(1));
	vTaskDelay(100 / portTICK_PERIOD_MS);

	//Send all the commands
	uint16_t cmd = 0;
    while (init_cmds[cmd].databytes != 0xff)
	{
		DEV_RETURN_ON_ERROR_SILENT(st7796s_send_cmd(init_cmds[cmd].cmd));
		DEV_RETURN_ON_ERROR_SILENT(st7796s_send_data(init_cmds[cmd].data, init_cmds[cmd].databytes & 0x1F));
		if (init_cmds[cmd].databytes & 0x80)
		{
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}
		cmd++;
	}


	DEV_RETURN_ON_ERROR_SILENT(st7796s_set_orientation(0));
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_cmd(0x20));		//ST7796S_INVERT_COLORS 0x21 or 0x20

	DeviceSetStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}

DeviceResult ST7796S::DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  	DEV_RETURN_ON_ERROR_SILENT(SetWindow(x, y, x + 1, y + 1));
  	DEV_RETURN_ON_ERROR_SILENT(WriteWindow(&color, 1));
    return DeviceResult::Ok;
}

DeviceResult ST7796S::SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    ContextLock lock(mutex);
    uint8_t data[4];
	/*Column addresses*/
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_cmd(0x2A));
	data[0] = (x1 >> 8) & 0xFF;
	data[1] = x1 & 0xFF;
	data[2] = (x2 >> 8) & 0xFF;
	data[3] = x2 & 0xFF;
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_data(data, 4));

	/*Page addresses*/
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_cmd(0x2B));
	data[0] = (y1 >> 8) & 0xFF;
	data[1] = y1 & 0xFF;
	data[2] = (y2 >> 8) & 0xFF;
	data[3] = y2 & 0xFF;
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_data(data, 4));
    return DeviceResult::Ok;
}

DeviceResult ST7796S::WriteWindow(uint16_t * colors, size_t size)
{
    ContextLock lock(mutex);
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_cmd(0x2C));
	DEV_RETURN_ON_ERROR_SILENT(st7796s_send_color(colors, size));
    return DeviceResult::Ok;
}

DeviceResult ST7796S::SetBacklight(bool value)
{
    ContextLock lock(mutex);
    return blckPin.GpioPinWrite(value);
}


