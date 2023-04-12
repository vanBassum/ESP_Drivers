#include "st7735.h"
#include <vector>
#include "rom/gpio.h"

#define DELAY 0x80
#define HAL_MAX_DELAY	  100


#define AST7735_MADCTL_MY   0x80
#define AST7735_MADCTL_MX   0x40
#define AST7735_MADCTL_MV   0x20
#define AST7735_MADCTL_ML   0x10
#define AST7735_MADCTL_RGB  0x00
#define AST7735_MADCTL_BGR  0x08
#define AST7735_MADCTL_MH   0x0

#define ST7735_ROTATION (AST7735_MADCTL_MX | AST7735_MADCTL_MY | AST7735_MADCTL_BGR)

#define ST7735_NOP     0
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09
#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E
#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD
#define ST7735_PWCTR6  0xFC
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF
#define ST7735_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define ST7735_IS_128X128

// based on Adafruit ST7735 library for Arduino
static const uint8_t
  init_cmds1[] = {            // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      ST7735_ROTATION,        //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 },                 //     16-bit color

#if (defined(ST7735_IS_128X128) || defined(ST7735_IS_160X128))
  init_cmds2[] = {            // Init for 7735R, part 2 (1.44" display)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F },           //     XEND = 127
#endif // ST7735_IS_128X128

#ifdef ST7735_IS_160X80
  init_cmds2[] = {            // Init for 7735S, part 2 (160x80 display)
    3,                        //  3 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x4F,             //     XEND = 79
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F ,            //     XEND = 159
    ST7735_INVON, 0 },        //  3: Invert colors
#endif

  init_cmds3[] = {            // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay



esp_err_t ST7735::Init(SPIBus* spiBus)
{
    esp_err_t result = ESP_FAIL;
	gpio_pad_select_gpio(settings.dc);
    gpio_set_direction(settings.cs, GPIO_MODE_OUTPUT);
    gpio_set_direction(settings.dc, GPIO_MODE_OUTPUT);
    gpio_set_direction(settings.rst, GPIO_MODE_OUTPUT);
    Reset();
    spi_device_interface_config_t devcfg =  {
      .clock_speed_hz = 20000000,
      .spics_io_num = settings.cs,
      .flags = SPI_DEVICE_NO_DUMMY,
      .queue_size = 7,
    };

    result = spidev.Init(spiBus, &devcfg);	

    ST7735_ExecuteCommandList(init_cmds1);
    ST7735_ExecuteCommandList(init_cmds2);
    ST7735_ExecuteCommandList(init_cmds3);

    //std::vector<uint8_t> initList;
    //CreateInitList(&initList);
    //ST7735_ExecuteCommandList(&initList[0]);
    return result;
}

void ST7735::WriteWindow(uint16_t *colors, size_t size)
{
	size_t bytes = size * 2;
	uint8_t* buf = (uint8_t*)colors;
	
	int i = 0;
	while (i < bytes)
	{
		size_t wrt = bytes - i;

		if (wrt > 1024)
			wrt = 1024;
		ST7735_WriteData(&buf[i], wrt);
		i += wrt;
	}
}



void ST7735::SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    // column address set
    ST7735_WriteCommand(ST7735_CASET);
	uint8_t xx0 = x0 + settings.xStart;
	uint8_t xx1 = x1 + settings.xStart;
	
    uint8_t data[] = { 0x00, xx0, 0x00, xx1 };
    ST7735_WriteData(data, sizeof(data));

    // row address set
    ST7735_WriteCommand(ST7735_RASET);
    data[1] = y0 + settings.yStart;
    data[3] = y1 + settings.yStart;
    ST7735_WriteData(data, sizeof(data));

    // write to RAM
    ST7735_WriteCommand(ST7735_RAMWR);
}


void ST7735::ST7735_WriteCommand(uint8_t cmd)
{
    gpio_set_level(settings.dc, 0);
    Transmit(&cmd, sizeof(cmd));
}



void ST7735::ST7735_WriteData(uint8_t *buff, size_t buff_size)
{
    gpio_set_level(settings.dc, 1);
    Transmit(buff, buff_size);
}

void ST7735::Transmit(uint8_t *data, size_t size)
{
    spi_transaction_t SPITransaction;
	esp_err_t ret;
	if (size > 0) {
		memset(&SPITransaction, 0, sizeof(spi_transaction_t));
		SPITransaction.length = size * 8;
		SPITransaction.tx_buffer = data;
		ret = spidev.Transmit(&SPITransaction);
		assert(ret == ESP_OK); 
	}
}

void ST7735::ST7735_ExecuteCommandList(const uint8_t *addr)
{
    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;
    while(numCommands--) {
        uint8_t cmd = *addr++;
        ST7735_WriteCommand(cmd);

        numArgs = *addr++;
        // If high bit set, delay follows args
        ms = numArgs & DELAY;
        numArgs &= ~DELAY;
        if(numArgs) {
            ST7735_WriteData((uint8_t*)addr, numArgs);
            addr += numArgs;
        }

        if(ms) {
            ms = *addr++;
            if(ms == 255) ms = 500;
	        vTaskDelay(ms / portTICK_PERIOD_MS);
        }
    }
}

void ST7735::Reset()
{
    gpio_set_level(settings.rst, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level(settings.rst, 1);
}



void ST7735::DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	if ((x >= settings.width) || (y >= settings.height))
		return;


	SetWindow(x, y, x + 1, y + 1);
	uint8_t data[] = { 
		(uint8_t)(color >> 8), 
		(uint8_t)(color & 0xFF)
	 };
	ST7735_WriteData(data, sizeof(data));

}

void ST7735::CreateInitList(std::vector<uint8_t> *list)
{

    static const uint8_t init_cmds1[] = {            // Init for 7735R, part 1 (red or green tab)
        15,                       // 15 commands in list:
        ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
        150,                    //     150 ms delay
        ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
        255,                    //     500 ms delay
        ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
        0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
        0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
        0x01, 0x2C, 0x2D,       //     Dot inversion mode
        0x01, 0x2C, 0x2D,       //     Line inversion mode
        ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
        0x07,                   //     No inversion
        ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
        0xA2,
        0x02,                   //     -4.6V
        0x84,                   //     AUTO mode
        ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
        0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
        ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
        0x0A,                   //     Opamp current small
        0x00,                   //     Boost frequency
        ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
        0x8A,                   //     BCLK/2, Opamp current small & Medium low
        0x2A,  
        ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
        0x8A, 0xEE,
        ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
        0x0E,
        ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
        ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
        (ST7735_MADCTL_MY | ST7735_MADCTL_MV),        //     row addr/col addr, bottom to top refresh
        ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
        0x05 };                 //     16-bit color


    static const uint8_t init_cmds2A[] = {            // Init for 7735R, part 2 (1.44" display)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
    0x00, 0x00,             //     XSTART = 0
    0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
    0x00, 0x00,             //     XSTART = 0
    0x00, 0x7F };           //     XEND = 127


  static const uint8_t init_cmds2B[] = {            // Init for 7735S, part 2 (160x80 display)
    3,                        //  3 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x4F,             //     XEND = 79
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F ,            //     XEND = 159
    ST7735_INVON, 0 };        //  3: Invert colors


  static const uint8_t init_cmds3[] = {            // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay




    list->insert(list->end(), &init_cmds1[0], &init_cmds1[sizeof(init_cmds1)]);


    if(     (settings.width == 128 && settings.height == 128)
        || (settings.width == 160 && settings.height == 128))
    {
        list->insert(list->end(), &init_cmds2A[0], &init_cmds2A[sizeof(init_cmds2A)]);
    }


    if( settings.width == 160 && settings.height == 80)
    {
        list->insert(list->end(), &init_cmds2B[0], &init_cmds2B[sizeof(init_cmds2B)]);
    }

    list->insert(list->end(), &init_cmds3[0], &init_cmds3[sizeof(init_cmds3)]);


}




