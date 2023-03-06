#pragma once
#include "esp_system.h"
#include "esp_com.h"
#include "esp_base.h"

using namespace ESP_Com;
using namespace ESP_Base;

namespace ESP_Drivers
{
    class ST7735
    {
        SPIDevice spidev;

        void CreateInitList(std::vector<uint8_t>* list);
        void ST7735_WriteCommand(uint8_t cmd);
        void ST7735_WriteData(uint8_t* buff, size_t buff_size);
        void ST7735_ExecuteCommandList(const uint8_t *addr);
        void Reset();
        void Transmit(uint8_t* data, size_t size);
    public:
        enum Options
        {
            ST7735_MADCTL_MY  = 0x80,
            ST7735_MADCTL_MX  = 0x40,
            ST7735_MADCTL_MV  = 0x20,
            ST7735_MADCTL_ML  = 0x10,
            ST7735_MADCTL_RGB = 0x00,
            ST7735_MADCTL_BGR = 0x08,
            ST7735_MADCTL_MH  = 0x04,
        };
        //DEFINE_ENUM_FLAG_OPERATORS(Options)

        struct Settings
        {
            uint16_t width = 128;
            uint16_t height = 128;
            uint16_t xStart = 2;
            uint16_t yStart = 3;
        	Options options = (Options)((int)ST7735_MADCTL_MX | (int)ST7735_MADCTL_MY | (int)ST7735_MADCTL_BGR);
            gpio_num_t cs = GPIO_NUM_NC;
            gpio_num_t dc = GPIO_NUM_NC;
            gpio_num_t rst = GPIO_NUM_NC;
            transaction_cb_t pre_cb  = NULL;
            transaction_cb_t post_cb = NULL;
        };

        Settings settings;
        esp_err_t Init(SPIBus* spiBus);
        void DrawPixel(uint16_t x, uint16_t y, uint16_t color);
        void SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
		void WriteWindow(uint16_t* colors, size_t size);
    };
}



/*
void SetWritePinCallback(void(*writePin)(gpio_num_t pin, bool state));
void SetTransmitCallback(void(*transmit)(uint8_t* data, size_t size, int delay));




// call before initializing any SPI devices
void ST7735_Unselect();
void ST7735_Init(void);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7735_FillScreen(uint16_t color);
void ST7735_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ST7735_InvertColors(bool invert);

*/
