#pragma once
#include "IDevice.h"

enum SPIFlags
{
    NONE    = 0x00,
    POLLED  = 0x01,
};

class ISpiDevice : public IDevice {
public:
    virtual ~ISpiDevice() {}
    virtual DeviceResult SpiTransmit(const uint8_t* txData, uint8_t* rxData, size_t size, SPIFlags flags = SPIFlags::NONE) = 0;    //data does accept nullptr
};

