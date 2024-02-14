#pragma once

#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <iostream>
#include <cstdint>
#include <functional>
#include <memory>
#include <cstring>
#include "esp_check.h"

#define DEV_SET_STATUS_AND_RETURN_ON_FALSE(a, newStatus, returnValue, log_tag, format, ...) do { \
        if (unlikely(!(a))) {                                                                   \
            ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__);        \
            setStatus(newStatus);                                                               \
            return returnValue;                                                                 \
        }                                                                                       \
    } while(0)

#define DEV_SET_STATUS_AND_RETURN_ON_TRUE(a, newStatus, returnValue, log_tag, format, ...) do {  \
        if (unlikely(a)) {                                                                      \
            ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__);        \
            setStatus(newStatus);                                                               \
            return returnValue;                                                                 \
        }                                                                                       \
    } while(0)

#define DEV_RETURN_ON_FALSE(a, returnValue, log_tag, format, ...) do {                          \
        if (unlikely(!(a))) {                                                                   \
            ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__);        \
            return returnValue;                                                                 \
        }                                                                                       \
    } while(0)


#define DEV_RETURN_ON_TRUE(a, returnValue, log_tag, format, ...) do {                           \
        if (unlikely(a)) {                                                                      \
            ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__);        \
            return returnValue;                                                                 \
        }                                                                                       \
    } while(0)

#define GET_STR_OR_RETURN(dst, property, newStatus, returnValue, log_tag, format, ...) do {     \
        const DevicePropertyValue* value = property;                                            \
        if (value == NULL) {                                                                    \
            setStatus(newStatus);                                                               \
            ESP_LOGE(log_tag, format, ##__VA_ARGS__);                                           \
            return returnValue;                                                                 \
        }                                                                                       \
        dst = value->str;                                                                       \
    } while (0)


#define GET_DEV_OR_RETURN(dst, device, newStatus, returnValue, log_tag, format, ...) do {       \
        if (device == NULL) {                                                                   \
            setStatus(newStatus);                                                               \
            ESP_LOGE(log_tag, format, ##__VA_ARGS__);                                           \
            return returnValue;                                                                 \
        }                                                                                       \
        dst = device;                                                                           \
    } while (0)




#define DEVICE_PROP_STR(val)  {.str = (val)} // Initialize a pointer to a string
#define DEVICE_PROP_I32(val)  {.i32 = (val)} // Initialize an int32_t value
#define DEVICE_END_MARKER  {nullptr, {0}}  // End marker for the device tree

union DevicePropertyValue {
    const char*  str;
    int32_t i32;
};

struct DeviceProperty {
    const char* key;
    DevicePropertyValue value;
};

using Device = const DeviceProperty*; 
using DeviceTree = const Device*;   

