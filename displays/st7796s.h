#pragma once
#include "esp_system.h"
#include "esp_base.h"
#include "spi.h"



class ST7796S
{
public:
    struct Config{
        gpio_num_t dc = GPIO_NUM_NC;
        gpio_num_t rst = GPIO_NUM_NC;
        gpio_num_t blck = GPIO_NUM_NC;
        uint16_t   hor_res = 320;
        uint16_t   ver_res = 480;
    };
private:
    Config config = {};
    std::shared_ptr<SPIDevice> spidev;
    bool initialized = false;
    void st7796s_sleep_in();
    void st7796s_sleep_out();
    void st7796s_send_cmd(uint8_t cmd);
    void st7796s_send_data(void *data, uint16_t length);
    void st7796s_send_color(void *data, uint16_t length);
    void st7796s_set_orientation(uint8_t orientation);
    void st7796s_spi_transfer(const uint8_t* txData, uint8_t* rxData, size_t length);
    
public:
	ST7796S(std::shared_ptr<SPIDevice> spidev);
    ST7796S() = default;

    void init(void);
    bool isInitialized() const;
    void setConfig(const Config& newConfig);
    void DrawPixel(uint16_t x, uint16_t y, uint16_t color);
    void SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void WriteWindow(uint16_t* colors, size_t size);
    uint16_t getHeight();
    uint16_t getWidth() ;
};
