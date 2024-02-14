#pragma once
#include "helpers.h"


class IDeviceConfig
{
public:
    virtual ~IDeviceConfig() {}
    virtual const DevicePropertyValue* getProperty(const char* key) = 0;
};

class DeviceConfigReader : public IDeviceConfig {
    Device deviceConfig;
public:
    // Constructor to initialize the device tree
    DeviceConfigReader(Device deviceConfig) : deviceConfig(deviceConfig) {}

    // Method to read properties of a device by key
    virtual const DevicePropertyValue* getProperty(const char* key) override {
        for (int i = 0; deviceConfig[i].key != nullptr; i++) {
            if (std::strcmp(deviceConfig[i].key, key) == 0) {
                return &deviceConfig[i].value;
            }
        }
        return nullptr; // Property not found
    }
};


