#pragma once
#include "helpers.h"
#include "DriverRegistry.h"
#include "IDeviceDetector.h"
#include "IDevice.h"

class DeviceManager : public std::enable_shared_from_this<DeviceManager> {
    std::shared_ptr<DriverRegistry> driverRegistry;
    std::vector<std::shared_ptr<IDeviceDetector>> detectors;
    std::vector<std::shared_ptr<IDevice>> devices;

    // Create a new device based on the configuration
    void CreateDevice(IDeviceConfig& config) {
        const char* deviceKey = config.getProperty("key")->str;

        if (deviceKey == nullptr) {
            ESP_LOGE("DeviceManager", "Device key not found");
            return;
        }

        // Check if a device with this key already exists
        if (std::any_of(devices.begin(), devices.end(), [deviceKey](const auto& device) {
            return std::strcmp(device->key, deviceKey) == 0;
        })) {
            //ESP_LOGW("DeviceManager", "Device with key %s already exists", deviceKey);
            return;
        }

        // Create a new device
        auto device = driverRegistry->CreateDriver(shared_from_this(), config);
        if(device)
        {
            devices.push_back(device);
            ESP_LOGI("DeviceManager", "Created device %s", deviceKey);
        }
    }

public:
    DeviceManager(std::shared_ptr<DriverRegistry> driverRegistry) : driverRegistry(driverRegistry) {};

    // Register a new detector
    void RegisterDetector(std::shared_ptr<IDeviceDetector> detector) {
        detectors.push_back(detector);
    }

    template<typename Device>
    std::shared_ptr<Device> getDeviceByKey(const char* key) {
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

    // Start working
    void work() {
        while (true) {
            // TODO: Loop through devices, depending on state:
            // TODO: - Check if devices need to be destroyed
            // TODO: - Check if devices need initialisation (in case initialisation wasn't possible before, e.g. missing dependencies or not ready)

            // Loop through detectors to find new devices
            for (auto& detector : detectors) {
                detector->search([this](IDeviceConfig& config) {
                    CreateDevice(config);
                });
            }

            // TODO: Each time we did something, skip this delay!
            vTaskDelay(1000); // Make this setting.
        }
    }
};
