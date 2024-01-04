#include "st7796s.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


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




/*********************
 *      DEFINES
 *********************/
#define TAG "ST7796S"

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



ST7796S::ST7796S(std::shared_ptr<SPIDevice> spidev) : spidev(spidev)
{
}


bool ST7796S::isInitialized() const
{
    return initialized;
}

void ST7796S::setConfig(const Config &newConfig)
{
	assert(!initialized);
	config = newConfig;
}


void ST7796S::init(void)
{
	assert(!initialized);

	lcd_init_cmd_t init_cmds[] = {
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

	//Initialize non-SPI GPIOs
	gpio_reset_pin(config.blck);
	gpio_set_direction(config.blck, GPIO_MODE_OUTPUT);
	gpio_set_level(config.blck, 1);

	gpio_reset_pin(config.dc);
	gpio_set_direction(config.dc, GPIO_MODE_OUTPUT);
	gpio_reset_pin(config.rst);
	gpio_set_direction(config.rst, GPIO_MODE_OUTPUT);

    //Reset the display
	gpio_set_level(config.rst, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(config.rst, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);


	ESP_LOGI(TAG, "Initialization.");

	//Send all the commands
	uint16_t cmd = 0;
    while (init_cmds[cmd].databytes != 0xff)
	{
		st7796s_send_cmd(init_cmds[cmd].cmd);
		st7796s_send_data(init_cmds[cmd].data, init_cmds[cmd].databytes & 0x1F);
		if (init_cmds[cmd].databytes & 0x80)
		{
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}
		cmd++;
	}

    //st7796s_enable_backlight(true);

	st7796s_set_orientation(0);

#if ST7796S_INVERT_COLORS == 1
	st7796s_send_cmd(0x21);
#else
	st7796s_send_cmd(0x20);
#endif
	initialized = true;
}


void ST7796S::DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	assert(initialized);
  	SetWindow(x, y, x + 1, y + 1);
  	WriteWindow(&color, 1);
}




void ST7796S::SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	assert(initialized);
    uint8_t data[4];
	/*Column addresses*/
	st7796s_send_cmd(0x2A);
	data[0] = (x1 >> 8) & 0xFF;
	data[1] = x1 & 0xFF;
	data[2] = (x2 >> 8) & 0xFF;
	data[3] = x2 & 0xFF;
	st7796s_send_data(data, 4);

	/*Page addresses*/
	st7796s_send_cmd(0x2B);
	data[0] = (y1 >> 8) & 0xFF;
	data[1] = y1 & 0xFF;
	data[2] = (y2 >> 8) & 0xFF;
	data[3] = y2 & 0xFF;
	st7796s_send_data(data, 4);
}

void ST7796S::WriteWindow(uint16_t* colors, size_t size)
{
	assert(initialized);
	/*Memory write*/
	st7796s_send_cmd(0x2C);
	st7796s_send_color((void *)colors, size * 2);
}

uint16_t ST7796S::getHeight()
{
	assert(initialized);
    return config.ver_res;
}

uint16_t ST7796S::getWidth()
{
	assert(initialized);
    return config.hor_res;
}

void ST7796S::st7796s_sleep_in()
{
	uint8_t data[] = {0x08};
	st7796s_send_cmd(0x10);
	st7796s_send_data(&data, 1);
}

void ST7796S::st7796s_sleep_out()
{
	uint8_t data[] = {0x08};
	st7796s_send_cmd(0x11);
	st7796s_send_data(&data, 1);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
void disp_wait_for_pending_transactions(void)
{
    //spi_transaction_t *presult;
//
	//while(uxQueueMessagesWaiting(TransactionPool) < SPI_TRANSACTION_POOL_SIZE) {	/* service until the transaction reuse pool is full again */
    //    if (spi_device_get_trans_result(spi, &presult, 1) == ESP_OK) {
	//		xQueueSend(TransactionPool, &presult, portMAX_DELAY);
    //    }
    //}
}


void ST7796S::st7796s_send_cmd(uint8_t cmd)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(config.dc, 0); /*Command mode*/
    st7796s_spi_transfer(&cmd, nullptr, 1);
}

void ST7796S::st7796s_send_data(void *data, uint16_t length)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(config.dc, 1); /*Data mode*/
    st7796s_spi_transfer((uint8_t*)data, nullptr, length);
}

void ST7796S::st7796s_send_color(void *data, uint16_t length)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(config.dc, 1); /*Data mode*/
    st7796s_spi_transfer((uint8_t*)data, nullptr, length);
}

void ST7796S::st7796s_spi_transfer(const uint8_t* txData, uint8_t* rxData, size_t length)
{
	if(length == 0)
	{
		return;
		ESP_LOGE("TEST", "tx = %p, rx = %p, length = %d", txData, rxData, (int)length);
	}
		
	spi_transaction_t transaction = {0};
	transaction.length = length * 8; // In bits
	transaction.tx_buffer = txData;
	transaction.rx_buffer = rxData;
	spidev->PollingTransmit(&transaction);
}


void ST7796S::st7796s_set_orientation(uint8_t orientation)
{
	const char *orientation_str[] = {"PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"};
	uint8_t data[] = {0x48, 0x88, 0x28, 0xE8};

	ESP_LOGI(TAG, "Display orientation: %s", orientation_str[orientation]);
	ESP_LOGI(TAG, "0x36 command value: 0x%02X", data[orientation]);

	st7796s_send_cmd(0x36);
	st7796s_send_data((void *)&data[orientation], 1);
}



