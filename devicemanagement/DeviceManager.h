#pragma once
#include "helpers.h"
#include "DriverRegistry.h"
#include "IDeviceDetector.h"
#include "IDevice.h"
#include "kernel.h"

class DeviceManager : public std::enable_shared_from_this<DeviceManager> {
    constexpr static const char* TAG = "DeviceManager";
public:
    struct Config {
        TickType_t scanInterval = pdMS_TO_TICKS(1000);
    };

private:
    Task task;
    Config config_;
    bool initialized_ = false;
    std::shared_ptr<DriverRegistry> driverRegistry;
    std::vector<std::shared_ptr<IDeviceDetector>> detectors;
    std::vector<std::shared_ptr<IDevice>> devices;

    // Create a new device based on the configuration
    bool CreateDevice(IDeviceConfig& config) {
        const char* deviceKey = nullptr;
        if (!config.getProperty("key", &deviceKey)) {
            ESP_LOGE(TAG, "Device key not found");
            return false;
        }

        // Check if a device with this key already exists
        if (std::any_of(devices.begin(), devices.end(), [deviceKey](const auto& device) {
            return std::strcmp(device->key, deviceKey) == 0;
        })) {
            //ESP_LOGW(TAG, "Device with key %s already exists", deviceKey);
            return false;
        }

        // Create a new device
        ESP_LOGI(TAG, "Creating device '%s'", deviceKey);
        auto device = driverRegistry->CreateDriver(shared_from_this(), config);
        if(device)
        {
            devices.push_back(device);
            return true;
        }
        return false;
    }

    int pollDevice(std::shared_ptr<IDevice> device)
    {
        switch (device->DeviceGetStatus())
        {
        case DeviceStatus::Dependencies: 
            return device->DeviceLoadDependencies(shared_from_this()) == DeviceResult::Ok ? 1 : 0;

        case DeviceStatus::Initializing:
            return device->DeviceInit() == DeviceResult::Ok ? 1 : 0;

        case DeviceStatus::Error:            // Driver is end of life
            return 2;

        case DeviceStatus::ConfigError:      // Unrecoverable error!
            ESP_LOGE(TAG, "Error in device configuration!");
            assert(false);
        case DeviceStatus::Created:          // This should not be possible, CreateDriver configures the device!
        case DeviceStatus::Ready:
        default:
            return 0;
        }
    }

    void work() {
        while (true) {
            bool reScan = false;

            for (auto it = devices.begin(); it != devices.end();) {
                int stat = pollDevice(*it);

                if (stat == 1) {
                    reScan = true;
                    ++it; // Move to the next device
                } else if (stat == 2) {
                    it = devices.erase(it); // Remove the device from the list
                } else {
                    ++it; // Move to the next device
                }
            }

            // Loop through detectors to find new devices
            for (auto& detector : detectors) {
                detector->search([&](IDeviceConfig& config) {
                    reScan |= CreateDevice(config);
                });
            }

            if(!reScan)
                vTaskDelay(config_.scanInterval);
        }
    }

public:
    DeviceManager(std::shared_ptr<DriverRegistry> driverRegistry) : driverRegistry(driverRegistry) {};

    void setConfig(const Config& newConfig) {
        assert(!initialized_ && "Config cannot be changed after initialization");
        config_ = newConfig;
    }

    void init() {
        assert(!initialized_ && "Already initialized");
        initialized_ = true;
        task.Init(TAG, 7, 1024 * 4);
        task.SetHandler([&](){work();});
        task.Run();
    }
    
    bool isInitialized() const {
        return initialized_;
    }

    // Register a new detector
    void RegisterDetector(std::shared_ptr<IDeviceDetector> detector) {
        assert(initialized_);
        detectors.push_back(detector);
    }

    template<typename Device>
    std::shared_ptr<Device> getDeviceByKey(const char* key) {
        assert(initialized_);
        auto it = std::find_if(devices.begin(), devices.end(), [key](const auto& device) {
            return std::strcmp(device->key, key) == 0;
        });

        if (it != devices.end()) {
            // Attempt to cast to the specified Device type TODO: Add typeinformation to IDevice to prevent problems!
            std::shared_ptr<Device> castedDevice = std::static_pointer_cast<Device>(*it);
            return castedDevice;
        } else {
            return nullptr;
        }
    }
};
