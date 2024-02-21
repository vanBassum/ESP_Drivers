#pragma once
#include "IDevice.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "rtos.h"

class SpiBus : public IDevice
{
    constexpr static const char *TAG = "SpiBus";
    spi_host_device_t host = SPI_HOST_MAX;
    int dmaChannel = -1;
    spi_bus_config_t busConfig = {};
    Mutex mutex;

public:
    virtual ~SpiBus() {}

    // Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual DeviceResult DeviceSetConfig(IDeviceConfig &config) override
    {
        ContextLock lock(mutex);
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("host", (int32_t *)&host), DeviceStatus::Error, DeviceResult::Error, TAG, "No property found for 'host'");
        config.getProperty("dmaChannel", (int32_t *)&dmaChannel);
        config.getProperty("mosi_io_num", (int32_t *)&busConfig.mosi_io_num);
        config.getProperty("miso_io_num", (int32_t *)&busConfig.miso_io_num);
        config.getProperty("sclk_io_num", (int32_t *)&busConfig.sclk_io_num);
        config.getProperty("quadwp_io_num", (int32_t *)&busConfig.quadwp_io_num);
        config.getProperty("quadhd_io_num", (int32_t *)&busConfig.quadhd_io_num);
        config.getProperty("data4_io_num", (int32_t *)&busConfig.data4_io_num);
        config.getProperty("data5_io_num", (int32_t *)&busConfig.data5_io_num);
        config.getProperty("data6_io_num", (int32_t *)&busConfig.data6_io_num);
        config.getProperty("data7_io_num", (int32_t *)&busConfig.data7_io_num);
        config.getProperty("max_transfer_sz", (int32_t *)&busConfig.max_transfer_sz);
        config.getProperty("flags", (int32_t *)&busConfig.flags);
        config.getProperty("intr_flags", (int32_t *)&busConfig.intr_flags);

        DeviceSetStatus(DeviceStatus::Dependencies);
        return DeviceResult::Ok;
    }

    // Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver. Also assume the devicemanger is not null.
    virtual DeviceResult DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override
    {
        ContextLock lock(mutex);
        DeviceSetStatus(DeviceStatus::Initializing);
        return DeviceResult::Ok;
    }

    // Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual DeviceResult DeviceInit() override
    {
        ContextLock lock(mutex);
        if(spi_bus_initialize(host, &busConfig, dmaChannel) != ESP_OK)
        {
            DeviceSetStatus(DeviceStatus::Error);
            return DeviceResult::Error;
        }
        DeviceSetStatus(DeviceStatus::Ready);
        return DeviceResult::Ok;
    }

    DeviceResult GetHost(spi_host_device_t* host)
    {
        ContextLock lock(mutex);
        DEV_RETURN_ON_FALSE(DeviceCheckStatus(DeviceStatus::Ready), DeviceResult::Error, TAG, "Driver not ready, status %d", (int)DeviceGetStatus());
        *host = this->host;
        return DeviceResult::Ok;
    }
};

// #include "driver/spi_master.h"
// #include "driver/gpio.h"
// #include <vector>
// #include <functional>
// class SPIDevice;
// class SPIBus
//{
//	static constexpr const char* TAG = "SPIBus";
//
// public:
//     struct Config
//     {
//		spi_host_device_t host = SPI_HOST_MAX;
//		int dmaChannel = -1;
//		spi_bus_config_t config = {};
//
//     };
//
//     SPIBus() = default;
//     ~SPIBus() = default;
//     void setConfig(const Config &newConfig);
//     void init();
//     bool isInitialized() const;
//
// private:
//     Config config = {}; // Default initialize Config
//     bool initialized = false;
//	friend SPIDevice;
// };
