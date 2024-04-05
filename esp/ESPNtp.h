#pragma once
#include "rtos.h"
#include "esp_base.h"
#include "INtp.h"
#include "esp_sntp.h"
#include <sys/time.h>
#include "devicemanagement.h"
#include "ESPWifi.h"
#include "lan87xx.h"

class ESPNtp : public INtp
{
    constexpr static const char* TAG = "ESPNtp";
	Mutex mutex;

    // Dependencies:
    // must wait for ethernet of wifi initalized


    std::shared_ptr<ESPWifi> wifi;
    std::shared_ptr<LAN87xx> lan;
    std::shared_ptr<DeviceManager> deviceManager;

    static std::function<void(DateTime newTime)> callback;

    static void timeSyncCallback(struct timeval *tv);

	//Config:
    const char* server;


public:

    ESPNtp(std::shared_ptr<DeviceManager> deviceManager);
	virtual Result DeviceSetConfig(IDeviceConfig& config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    virtual Result NtpOnSyncSetCallback(std::function<void(DateTime newTime)> callback) override;

};


