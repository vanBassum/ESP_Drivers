#pragma once
#include "IDevice.h"

class IStream : public IDevice {
public:
    virtual ~IStream() {}

    /// @brief          Blocking call, writes data to stream
    /// @param data     Buffer with data to send
    /// @param length   Size of data to send
    /// @param written  Number of bytes actually send, pass null if not used
    /// @param timeout  Max time to wait for data to be send
    /// @return 
    virtual Result StreamWrite(const uint8_t* data, size_t length, size_t* written = nullptr, TickType_t timeout = portMAX_DELAY ) { return Result::NotSupported; }

    /// @brief          Blocking call, reads data from stream.
    /// @param data     Buffer for received data
    /// @param length   Size of data buffer
    /// @param read     Actual number of bytes read, pass null if not used
    /// @param timeout  Max time to wait for data to be read
    /// @return         
    virtual Result StreamRead(uint8_t* data, size_t length, size_t* read = nullptr, TickType_t timeout = portMAX_DELAY) { return Result::NotSupported; }

};


