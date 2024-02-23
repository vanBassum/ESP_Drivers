#pragma once
#include "esp_base.h"
#include "IStream.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"


class TCPClient : public IStream
{
    constexpr static const char* TAG = "TCPClient";
	Mutex mutex;
    Socket socket;
public:

	virtual Result DeviceSetConfig(IDeviceConfig& config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    Result Connect(Endpoint& endpoint);
    virtual Result StreamWrite(const uint8_t* data, size_t length, size_t* written = nullptr, TickType_t timeout = portMAX_DELAY ) override;     
    virtual Result StreamRead(uint8_t* data, size_t length, size_t* read = nullptr, TickType_t timeout = portMAX_DELAY) override;     
};

