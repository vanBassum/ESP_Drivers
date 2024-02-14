#pragma once
#include "IDevice.h"


class ISpiDevice : public IDevice {
public:
    virtual ~ISpiDevice() {}
    virtual ErrCode Write(uint8_t* data, size_t size) = 0;
    virtual ErrCode Read(uint8_t* data, size_t size) = 0;
};