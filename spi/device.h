#pragma once
#include "ISpiDevice.h"
#include "bus.h"
#include "DeviceManager.h"

#define SPI_CS_PIN_A0 						GPIO_NUM_5
#define SPI_CS_PIN_A1 						GPIO_NUM_32
#define SPI_CS_PIN_A2 						GPIO_NUM_2


//TODO: use gpio device to select cs pin!!!
void IRAM_ATTR Select(spi_transaction_t* t);

//TODO: use gpio device to select cs pin!!!
void IRAM_ATTR Deselect(spi_transaction_t* t);

class SpiDevice : public ISpiDevice {
    Mutex mutex;
    constexpr static const char* TAG = "SpiDevice";
    const char* spiBusKey = nullptr;
    std::shared_ptr<SpiBus> spiBus;
    spi_device_handle_t handle = NULL;
    spi_device_interface_config_t devConfig = {};

public:
    uint8_t customCsPin = 0;

    virtual ~SpiDevice() {}

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual ErrCode setConfig(IDeviceConfig& config) override  
    {
        ContextLock lock(mutex);
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiBus", &spiBusKey), Status::ConfigError, ErrCode::ConfigError, TAG, "No property found for 'spiBus'");
        config.getProperty("command_bits", &devConfig.command_bits);
        config.getProperty("address_bits", &devConfig.address_bits);
        config.getProperty("dummy_bits", &devConfig.dummy_bits);
        config.getProperty("mode", &devConfig.mode);
        config.getProperty("duty_cycle_pos", &devConfig.duty_cycle_pos);
        config.getProperty("cs_ena_pretrans", &devConfig.cs_ena_pretrans);
        config.getProperty("cs_ena_posttrans", &devConfig.cs_ena_posttrans);
        config.getProperty("clock_speed_hz", &devConfig.clock_speed_hz);
        config.getProperty("input_delay_ns", &devConfig.input_delay_ns);
        config.getProperty("spics_io_num", &devConfig.spics_io_num);
        config.getProperty("flags", &devConfig.flags);
        config.getProperty("queue_size", &devConfig.queue_size);


        if(config.getProperty("customCS", &customCsPin))
        {
            devConfig.pre_cb = Select;
            devConfig.post_cb = Deselect;
            
        }

        //config.getProperty("pre_cb", &devConfig.pre_cb);
        //config.getProperty("post_cb", &devConfig.post_cb);



        
        setStatus(Status::Dependencies);
        return ErrCode::Ok;
    }

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver. Also assume the devicemanger is not null.
    virtual ErrCode loadDependencies(std::shared_ptr<DeviceManager> deviceManager) override
    {
        ContextLock lock(mutex);
        GET_DEV_OR_RETURN(spiBus, deviceManager->getDeviceByKey<SpiBus>(spiBusKey), Status::Dependencies, ErrCode::Dependency, TAG, "Dependencies not ready %d", (int)getStatus());
        setStatus(Status::Initializing);
        return ErrCode::Ok;
    }

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual ErrCode init() override
    {
        ContextLock lock(mutex);
        spi_host_device_t host;
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiBus->GetHost(&host) == ErrCode::Ok, Status::Dependencies, ErrCode::Dependency, TAG, "spiBus->GetHost error");

        if(spi_bus_add_device(host, &devConfig, &handle) != ESP_OK)
        {
            setStatus(Status::Error);
            return ErrCode::InitFault;
        }

        //TODO: Initialize the driver here! Dont forget to check the result of the functions you call from the dependecies.
        setStatus(Status::Ready);
        return ErrCode::Ok;
    }

    virtual ErrCode Write(uint8_t* data, size_t size) override 
    {
        ContextLock lock(mutex);
        //Ensure the device is in the ready state
        DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());

        //TODO: Implement writing of data. Dont forget to check the result of the functions you call from the dependecies.
        return ErrCode::Ok;
    }

    virtual ErrCode Read(uint8_t* data, size_t size) override 
    {
        ContextLock lock(mutex);
        //Ensure the device is in the ready state
        DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());

        //TODO: Implement reading of data. Dont forget to check the result of the functions you call from the dependecies.
        return ErrCode::Ok;
    }

    ErrCode Transmit(uint8_t* txData, uint8_t* rxData, size_t size) 
    {
        ContextLock lock(mutex);
        //Ensure the device is in the ready state
        DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());

        //TODO: Implement reading of data. Dont forget to check the result of the functions you call from the dependecies.
        return ErrCode::Ok;
    }
};


//TODO: use gpio device to select cs pin!!!
void IRAM_ATTR Select(spi_transaction_t* t)
{
    //SpiDevice* device = (SpiDevice*)t->user;    
    gpio_set_level(SPI_CS_PIN_A0, 0);
	gpio_set_level(SPI_CS_PIN_A1, 0);
	gpio_set_level(SPI_CS_PIN_A2, 0);
}

//TODO: use gpio device to select cs pin!!!
void IRAM_ATTR Deselect(spi_transaction_t* t)
{
    SpiDevice* device = (SpiDevice*)t->user;  
    gpio_set_level(SPI_CS_PIN_A0, device->customCsPin & 0x01);
	gpio_set_level(SPI_CS_PIN_A1, device->customCsPin & 0x02);
	gpio_set_level(SPI_CS_PIN_A2, device->customCsPin & 0x04);
}



//#include "mutex.h"
//#include <memory>
//#include <functional>
//#include "bus.h"
//
//class SPIDevice {
//    static constexpr const char* TAG = "SPIDevice";
//    Mutex mutex;
//    spi_device_handle_t handle = NULL;
//    std::shared_ptr<SPIBus> spiBus;
//    bool initialized = false;
//
//public:
//    struct Config {
//        spi_device_interface_config_t devConfig = {};
//    };
//
//private:
//	Config config = {};
//
//public:
//    SPIDevice(std::shared_ptr<SPIBus> spiBus);
//    ~SPIDevice() = default;
//
//    void setConfig(const Config& newConfig);
//    void init();
//    void transfer(uint8_t* txData, uint8_t* rxData, size_t length);
//    void PollingTransmit(spi_transaction_t* transaction);
//    void Transmit(spi_transaction_t* transaction);
//    void AcquireBus();
//    void ReleaseBus();
//	bool isInitialized() const;
//};
