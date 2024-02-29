#include "Uart.h"


Result ESPUart::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	RETURN_ON_ERR(config.getProperty("portNo", &portNo));
	RETURN_ON_ERR(config.getProperty("txPin", (int*) &txPin));
	RETURN_ON_ERR(config.getProperty("rxPin", (int*) &rxPin));
	config.getProperty("bufferSize", (uint32_t*)&bufferSize);

    rxBuffer = std::make_unique<uint8_t[]>(bufferSize);
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result ESPUart::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result ESPUart::DeviceInit()
{
	ContextLock lock(mutex);
	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
}


Result ESPUart::UartConfigure(const UartConfig *config)
{
    ContextLock lock(mutex);
    RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));
    uart_config_t uart_config = {0};
    uart_config.baud_rate = config->baudrate;

    switch (config->parity)
    {
    case UartConfigParity::UART_CFG_PARITY_EVEN:
        uart_config.parity = UART_PARITY_EVEN;
        break;
    case UartConfigParity::UART_CFG_PARITY_ODD:
        uart_config.parity = UART_PARITY_ODD;
        break;
    case UartConfigParity::UART_CFG_PARITY_NONE:
        uart_config.parity = UART_PARITY_DISABLE;
        break;
    default:
        return Result::NotSupported;
    }

    switch (config->stopBits)
    {
    case UartConfigStopBits::UART_CFG_STOP_BITS_1:
        uart_config.stop_bits = UART_STOP_BITS_1;
        break;
    case UartConfigStopBits::UART_CFG_STOP_BITS_1_5:
        uart_config.stop_bits = UART_STOP_BITS_1_5;
        break;
    case UartConfigStopBits::UART_CFG_STOP_BITS_2:
        uart_config.stop_bits = UART_STOP_BITS_2;
        break;
    default:
        return Result::NotSupported;
    }


    switch (config->dataBits)
    {
    case UartConfigDataBits::UART_CFG_DATA_BITS_5:
        uart_config.data_bits = UART_DATA_5_BITS;
        break;
    case UartConfigDataBits::UART_CFG_DATA_BITS_6:
        uart_config.data_bits = UART_DATA_6_BITS;
        break;
    case UartConfigDataBits::UART_CFG_DATA_BITS_7:
        uart_config.data_bits = UART_DATA_7_BITS;
        break;
    case UartConfigDataBits::UART_CFG_DATA_BITS_8:
        uart_config.data_bits = UART_DATA_8_BITS;
        break;
    default:
        return Result::NotSupported;
    }

    switch (config->flowCtrl)
    {
    case UartConfigFlowControl::UART_CFG_FLOW_CTRL_NONE:
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        break;
    case UartConfigFlowControl::UART_CFG_FLOW_CTRL_RTS_CTS:
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS;
        break;
    default:
        return Result::NotSupported;
    }

    if(uart_param_config(portNo, &uart_config) != ESP_OK) return Result::Error;
    if(uart_set_pin(portNo, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) return Result::Error;
    if(uart_driver_install(portNo, bufferSize * 2, 0, 0, NULL, 0) != ESP_OK) return Result::Error;

    return Result::Ok;
}

Result ESPUart::StreamWrite(const uint8_t *data, size_t length, size_t *written, TickType_t timeout)
{
    ContextLock lock(mutex);
    RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));
    
    
    return Result::Ok;
}

Result ESPUart::StreamRead(uint8_t *data, size_t length, size_t *read, TickType_t timeout)
{
    ContextLock lock(mutex);
    RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));
    
    
    return Result::Ok;
}
