#pragma once
#include "IDevice.h"

class IGpio : public IDevice {
public:
    virtual ~IGpio() {}
    virtual ErrCode Write(uint32_t bank, uint8_t mask, uint8_t value) = 0;
    virtual ErrCode Read(uint32_t bank, uint8_t mask, uint8_t* value) = 0;
    virtual ErrCode SetOuput(uint32_t bank, uint8_t mask) = 0;
    virtual ErrCode SetInput(uint32_t bank, uint8_t mask) = 0;
};

