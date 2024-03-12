#pragma once
#include "rtos.h"
#include "esp_base.h"
#include "INtp.h"
#include "esp_sntp.h"
#include <sys/time.h>


class ESPNtp : public INtp
{
    constexpr static const char* TAG = "ESPNtp";
	Mutex mutex;
    static std::function<void(DateTime newTime)> callback;

    static void timeSyncCallback(struct timeval *tv);

	//Config:
    const char* server;


public:

	virtual Result DeviceSetConfig(IDeviceConfig& config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    virtual Result NtpOnSyncSetCallback(std::function<void(DateTime newTime)> callback) override;

};


