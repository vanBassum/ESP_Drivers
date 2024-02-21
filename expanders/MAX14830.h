#pragma once
#include "interfaces.h"
#include "DeviceManager.h"
#include "rtos.h"
#include "spi/device.h"
#include <functional>
#include <list>



class MAX14830 : public IGpio
{
    constexpr static const char *TAG = "MAX14830";
    Mutex mutex;

    // Dependencies:
    uint8_t isrPin;
    uint8_t isrPort;
    const char *isrDeviceKey = nullptr;
    std::shared_ptr<IGpio> isrDevice;

    const char *spiDeviceKey = nullptr;
    std::shared_ptr<SpiDevice> spiDevice; // TODO: Change this so it can use the ISpiDevice

    struct IsrHandle
    {
        MAX14830 *device;
        uint8_t port;
        uint8_t pin;
        std::function<void()> callback;
    };

    // Local variables
    static std::list<std::shared_ptr<IsrHandle>> callbacks; // TODO: This can be optimized by using an array like so: std::function<void()> callbacks[port][pin]
    uint8_t gpioConfBuffer[4] = {};
    uint8_t gpioDataBuffer[4] = {};
    uint8_t gpioIRQBuffer[4] = {};

    // Local functions
    Result Detect(bool *result);
    Result SetRefClock(uint32_t *clk, bool *clkError);
    Result regmap_write(uint8_t cmd, uint8_t value);
    Result regmap_read(uint8_t cmd, uint8_t *value);
    Result max310x_port_read(uint8_t port, uint8_t cmd, uint8_t *value);
    Result max310x_port_write(uint8_t port, uint8_t cmd, uint8_t value);
    Result max310x_port_update(uint8_t port, uint8_t cmd, uint8_t mask, uint8_t value);
    uint8_t max310x_update_best_err(uint64_t f, int64_t *besterr);
    Result Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t *cmdData, uint8_t count);
    Result Max14830_ReadBufferPolled(uint8_t cmd, uint8_t *cmdData, uint8_t *replyData, uint8_t count);
    void isr_handler();
    static void processIsr(void *pvParameter1, uint32_t ulParameter2);
    Result handleIRQForPort(uint8_t port);
    Result portInit(uint8_t port);
    Result readIsrRegisters(uint8_t port, uint8_t *isr, uint8_t *sts);
    Result max310x_get_ref_clk(uint32_t* refClk);
    Result max310x_set_baud(uint8_t port, uint32_t baud, uint32_t* actualBaud);

public:
    virtual Result DeviceSetConfig(IDeviceConfig &config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    virtual Result GpioConfigure(uint32_t port, uint8_t mask, const GpioConfig* config) override;
    virtual Result GpioRead(uint32_t port, uint8_t mask, uint8_t *value) override;
    virtual Result GpioWrite(uint32_t port, uint8_t mask, uint8_t value) override;
    virtual Result GpioIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback) override;
    virtual Result GpioIsrRemoveCallback(uint32_t port, uint8_t pin)  override;


    Result UartConfigure(uint8_t port, const UartConfig* config);
    Result StreamWrite(uint8_t port, const uint8_t* data, size_t length, size_t* written = nullptr, TickType_t timeout = portMAX_DELAY);
    Result StreamRead(uint8_t port, uint8_t* data, size_t length, size_t* read = nullptr, TickType_t timeout = portMAX_DELAY);


};




// QR!!!!


