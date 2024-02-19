#pragma once
#include "interfaces.h"	
#include "DeviceManager.h"
#include "kernel.h"
#include "spi/device.h"
#include <functional>
#include <list>

class MAX14830 : public IDevice
{
    constexpr static const char* TAG = "MAX14830";
	Mutex mutex;

	//Dependencies:
    uint8_t isrPin;
    uint8_t isrPort;
    const char* isrDeviceKey = nullptr;		
    std::shared_ptr<IGpio> isrDevice;	

    const char* spiDeviceKey = nullptr;		
    std::shared_ptr<SpiDevice> spiDevice;	   //TODO: Change this so it can use the ISpiDevice

    struct IsrHandle
    {
        MAX14830* device;
        uint8_t port;
        uint8_t pin;
        std::function<void()> callback;
    };

    //Local variables
    static std::list<std::shared_ptr<IsrHandle>> callbacks; //TODO: This can be optimized by using an array like so: std::function<void()> callbacks[port][pin]
    uint8_t gpioConfBuffer[4] = {};
	uint8_t gpioDataBuffer[4] = {};
	uint8_t gpioIRQBuffer[4]  = {};

    //Local functions
    DeviceResult Detect(bool* result);
	DeviceResult SetRefClock(uint32_t* clk, bool* clkError);
    DeviceResult regmap_write(uint8_t cmd, uint8_t value);
	DeviceResult regmap_read(uint8_t cmd, uint8_t* value);
    DeviceResult max310x_port_read(uint8_t port, uint8_t cmd, uint8_t* value);
    DeviceResult max310x_port_write(uint8_t port, uint8_t cmd, uint8_t value);
    DeviceResult max310x_port_update(uint8_t port, uint8_t cmd, uint8_t mask, uint8_t value);
    uint8_t max310x_update_best_err(uint64_t f, int64_t* besterr);
	DeviceResult Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t * cmdData, uint8_t count);
	DeviceResult Max14830_ReadBufferPolled(uint8_t cmd, uint8_t * cmdData, uint8_t * replyData, uint8_t count);
    void isr_handler();
    static void processIsr( void * pvParameter1, uint32_t ulParameter2 );
    DeviceResult handleIRQForPort(uint8_t port);
    DeviceResult portInit(uint8_t port);
	DeviceResult readIsrRegisters(uint8_t port, uint8_t* isr, uint8_t* sts);

public:
	virtual DeviceResult setDeviceConfig(IDeviceConfig& config) override;
    virtual DeviceResult loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult init() override;

    DeviceResult portConfigure(uint32_t port, uint8_t mask, const GpioConfig *config);
    DeviceResult SetPins(uint8_t port, uint8_t mask, uint8_t value);
    DeviceResult GetPins(uint8_t port, uint8_t mask, uint8_t* value);
    DeviceResult portIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback);
    DeviceResult portIsrRemoveCallback(uint32_t port, uint8_t pin);

};

