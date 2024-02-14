#pragma once
#include "helpers.h"

class IDeviceConfig
{
public:
    virtual ~IDeviceConfig() {}
    virtual bool getProperty(const char *key, const char **value) = 0;
    virtual bool getProperty(const char *key, uint8_t *value) = 0;
    virtual bool getProperty(const char *key, uint16_t *value) = 0;
    virtual bool getProperty(const char *key, uint32_t *value) = 0;
    virtual bool getProperty(const char *key, int8_t *value) = 0;
    virtual bool getProperty(const char *key, int16_t *value) = 0;
    virtual bool getProperty(const char *key, int32_t *value) = 0;
    virtual bool getProperty(const char *key, int *value) = 0;
};

class DeviceConfigReader : public IDeviceConfig
{
    Device deviceConfig;

    // Method to read properties of a device by key
    const DevicePropertyValue *getProperty(const char *key)
    {
        for (int i = 0; deviceConfig[i].key != nullptr; i++)
        {
            if (std::strcmp(deviceConfig[i].key, key) == 0)
            {
                return &deviceConfig[i].value;
            }
        }
        return nullptr; // Property not found
    }

public:
    // Constructor to initialize the device tree
    DeviceConfigReader(Device deviceConfig) : deviceConfig(deviceConfig) {}

    virtual bool getProperty(const char *key, const char **value) override
    {
        const DevicePropertyValue *v = getProperty(key);
        if (v == nullptr)
            return false;
        *value = v->str;
        return true;
    }

    virtual bool getProperty(const char *key, uint8_t *value) override
    {
        const DevicePropertyValue *v = getProperty(key);
        if (v == nullptr)
            return false;
        *value = v->u08;
        return true;
    }

    virtual bool getProperty(const char *key, uint16_t *value) override
    {
        const DevicePropertyValue *v = getProperty(key);
        if (v == nullptr)
            return false;
        *value = v->u16;
        return true;
    }

    virtual bool getProperty(const char *key, uint32_t *value) override
    {
        const DevicePropertyValue *v = getProperty(key);
        if (v == nullptr)
            return false;
        *value = v->u32;
        return true;
    }

    virtual bool getProperty(const char *key, int8_t *value) override
    {
        const DevicePropertyValue *v = getProperty(key);
        if (v == nullptr)
            return false;
        *value = v->i08;
        return true;
    }

    virtual bool getProperty(const char *key, int16_t *value) override
    {
        const DevicePropertyValue *v = getProperty(key);
        if (v == nullptr)
            return false;
        *value = v->i16;
        return true;
    }

    virtual bool getProperty(const char *key, int32_t *value) override
    {
        const DevicePropertyValue *v = getProperty(key);
        if (v == nullptr)
            return false;
        *value = v->i32;
        return true;
    }

    virtual bool getProperty(const char *key, int *value) override
    {
        const DevicePropertyValue *v = getProperty(key);
        if (v == nullptr)
            return false;
        *value = v->i32;
        return true;
    }
};
