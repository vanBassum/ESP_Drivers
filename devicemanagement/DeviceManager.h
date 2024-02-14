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
        const char* deviceKey = config.getProperty("key")->str;

        if (deviceKey == nullptr) {
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
        auto device = driverRegistry->CreateDriver(shared_from_this(), config);
        if(device)
        {
            devices.push_back(device);
            ESP_LOGI(TAG, "Created device %s", deviceKey);
            return true;
        }
        return false;
    }

    void work() {
        while (true) {
            bool reScan = false;

            for (auto it = devices.begin(); it != devices.end();) {
                auto& device = *it;
                if (device->checkStatus(IDevice::Status::Configured)) {
                    reScan |= device->init(shared_from_this()) == IDevice::ErrCode::Ok;  //If initialisation was ok, dont wait for rescan.
                    ++it; // Move to the next device
                } else if (device->checkStatus(IDevice::Status::Error)) {
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
