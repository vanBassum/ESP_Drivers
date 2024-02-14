#pragma once
#include "IDevice.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "kernel.h"

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
    virtual ErrCode setConfig(IDeviceConfig &config) override
    {
        ContextLock lock(mutex);
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("host", (int32_t *)&host), Status::ConfigError, ErrCode::ConfigError, TAG, "No property found for 'host'");
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

        setStatus(Status::Dependencies);
        return ErrCode::Ok;
    }

    // Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver. Also assume the devicemanger is not null.
    virtual ErrCode loadDependencies(std::shared_ptr<DeviceManager> deviceManager) override
    {
        ContextLock lock(mutex);
        setStatus(Status::Initializing);
        return ErrCode::Ok;
    }

    // Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual ErrCode init() override
    {
        ContextLock lock(mutex);
        if(spi_bus_initialize(host, &busConfig, dmaChannel) != ESP_OK)
        {
            setStatus(Status::Error);
            return ErrCode::InitFault;
        }
        setStatus(Status::Ready);
        return ErrCode::Ok;
    }

    ErrCode GetHost(spi_host_device_t* host)
    {
        ContextLock lock(mutex);
        DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());
        *host = this->host;
        return ErrCode::Ok;
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
