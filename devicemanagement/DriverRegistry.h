#pragma once
#include <functional>
#include <vector>
#include <memory>
#include "IDevice.h"


class DriverRegistry {
    constexpr static const char* TAG = "DriverRegistry";
    struct DriverEntry {
        const char* compatibility;
        std::function<std::shared_ptr<IDevice>()> factory;
    };

    std::vector<DriverEntry> drivers;

public:
    // Register a driver
    template<typename Concrete>
    void RegisterDriver(const char* compatibility) {
        drivers.push_back({compatibility, []() {
            return std::make_shared<Concrete>();
        }});
    }

    // Create a driver based on compatibility field
    std::shared_ptr<IDevice> CreateDriver(std::shared_ptr<DeviceManager> deviceManager, IDeviceConfig& config) {
        
        const char* deviceKey = nullptr;
        if (!config.getProperty("key", &deviceKey)) {
            ESP_LOGE(TAG, "Device key not found");
            return nullptr;
        }

        const char* compatibility = nullptr;
        if (!config.getProperty("compatible", &compatibility)) {
            ESP_LOGE(TAG, "Compatible key not found");
            return nullptr;
        }

        for (const auto& entry : drivers) {
            if (std::strcmp(entry.compatibility, compatibility) == 0) {
                auto device = entry.factory();
                if(device)
                {
                    device->key = deviceKey;
                    device->setConfig(config);
                }
                return device;
            }
        }

        ESP_LOGE(TAG, "No driver registered for the specified compatibility");
        return nullptr;
    }
};