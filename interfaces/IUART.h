#pragma once
#include "IStream.h"


#define UART_CREATE_CONFIG(baud, parity, stopbits, databits, flowctrl)  \
    {                                                                   \
        baud,                                                           \
        parity,                                                         \
        stopbits,                                                       \
        databits,                                                       \
        flowctrl                                                        \
    }


enum UartConfigParity
{
    UART_CFG_PARITY_NONE    = 0x00,
    UART_CFG_PARITY_ODD     = 0x01,
    UART_CFG_PARITY_EVEN    = 0x02,
    UART_CFG_PARITY_MARK    = 0x03,
    UART_CFG_PARITY_SPACE   = 0x04,
};

enum UartConfigStopBits
{
    UART_CFG_STOP_BITS_0_5  = 0x00,
    UART_CFG_STOP_BITS_1    = 0x01,
    UART_CFG_STOP_BITS_1_5  = 0x02,
    UART_CFG_STOP_BITS_2    = 0x03,
};

enum UartConfigDataBits
{
    UART_CFG_DATA_BITS_5  = 0x00,
    UART_CFG_DATA_BITS_6  = 0x01,
    UART_CFG_DATA_BITS_7  = 0x02,
    UART_CFG_DATA_BITS_8  = 0x03,
    UART_CFG_DATA_BITS_9  = 0x04,
};

enum UartConfigFlowControl
{
    UART_CFG_FLOW_CTRL_NONE     = 0x00,
    UART_CFG_FLOW_CTRL_RTS_CTS  = 0x01,
    UART_CFG_FLOW_CTRL_DTR_DSR  = 0x02,
    UART_CFG_FLOW_CTRL_RS485    = 0x03,
};

struct UartConfig
{
    uint32_t baudrate;
    UartConfigParity parity;
    UartConfigStopBits stopBits;
    UartConfigDataBits dataBits;
    UartConfigFlowControl flowCtrl; 
};

class IUART : public IStream {
public:
    virtual ~IUART() {}
    virtual DeviceResult UartConfigure(const UartConfig* config) = 0;

    /// @brief          Blocking call, writes data to stream
    /// @param data     Buffer with data to send
    /// @param length   Size of data to send
    /// @param written  Number of bytes actually send, pass null if not used
    /// @param timeout  Max time to wait for data to be send
    /// @return 
    virtual DeviceResult StreamWrite(const uint8_t* data, size_t length, size_t* written = nullptr, TickType_t timeout = portMAX_DELAY ) override { return DeviceResult::NotSupported; }

    /// @brief          Blocking call, reads data from stream.
    /// @param data     Buffer for received data
    /// @param length   Size of data buffer
    /// @param read     Actual number of bytes read, pass null if not used
    /// @param timeout  Max time to wait for data to be read
    /// @return         
    virtual DeviceResult StreamRead(uint8_t* data, size_t length, size_t* read = nullptr, TickType_t timeout = portMAX_DELAY) override { return DeviceResult::NotSupported; }

};




