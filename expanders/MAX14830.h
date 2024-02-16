#pragma once
#include "interfaces.h"	
#include "DeviceManager.h"
#include "kernel.h"
#include "spi/device.h"

class MAX14830 : public IDevice
{
    constexpr static const char* TAG = "MAX14830";
	Mutex mutex;

    enum Ports
	{
		NUM_0 = 0x0,
		NUM_1 = 0x1,
		NUM_2 = 0x2,
		NUM_3 = 0x3,
	};

	//Dependencies:
    uint8_t isrPin;
    uint8_t isrPort;
    const char* isrDeviceKey = nullptr;		
    std::shared_ptr<IGpio> isrDevice;	


    const char* spiDeviceKey = nullptr;		
    std::shared_ptr<SpiDevice> spiDevice;	

    //Local variables
    Task task;
    uint8_t gpioConfBuffer[4] = {};
	uint8_t gpioDataBuffer[4] = {};
	uint8_t gpioIRQBuffer[4]  = {};

    //Local functions
    void Work();
    DeviceResult Detect(bool* result);
	DeviceResult SetRefClock(uint32_t* clk, bool* clkError);
    DeviceResult regmap_write(uint8_t cmd, uint8_t value);
	DeviceResult regmap_read(uint8_t cmd, uint8_t* value);
    DeviceResult max310x_port_read(uint8_t port, uint8_t cmd, uint8_t* value);
    DeviceResult max310x_port_write(Ports port, uint8_t cmd, uint8_t value);
    uint8_t max310x_update_best_err(uint64_t f, int64_t* besterr);
	DeviceResult Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t * cmdData, uint8_t count);
	DeviceResult Max14830_ReadBufferPolled(uint8_t cmd, uint8_t * cmdData, uint8_t * replyData, uint8_t count);

public:
	virtual DeviceResult setDeviceConfig(IDeviceConfig& config) override;
    virtual DeviceResult loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult init() override;
};

