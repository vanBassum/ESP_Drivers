#include "ESPNtp.h"
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"


std::function<void(DateTime newTime)> ESPNtp::callback;

Result ESPNtp::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
    RETURN_ON_ERR(config.getProperty("server", &server));
    config.getProperty("wifi", &wifiDeviceKey);
    config.getProperty("lan", &lanDeviceKey);
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result ESPNtp::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
    RETURN_ON_ERR(deviceManager->getDeviceByKey<ESPWifi>(wifiDeviceKey, wifiDevice));
    RETURN_ON_ERR(deviceManager->getDeviceByKey<LAN87xx>(lanDeviceKey, lanDevice));
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

void ESPNtp::timeSyncCallback(timeval *tv)
{
    if(callback)
    {
        callback(DateTime::Now());
    }
}


Result ESPNtp::DeviceInit()
{
	ContextLock lock(mutex);
    Endpoint ep;
    ip_addr_t ip;
    DNSResolver resolver;

    //We need either wifi or lan before we can continue
    if(wifiDevice->DeviceGetStatus() != DeviceStatus::Ready && wifiDevice->DeviceGetStatus() != DeviceStatus::lanDevice)
        return Result::Error;

    RETURN_ON_ERR(resolver.Resolve(server, ep));
    RETURN_ON_ERR(ep.GetIP(&ip));
    sntp_init();
    sntp_setserver(0, &ip);
    sntp_set_time_sync_notification_cb(timeSyncCallback);
	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
}

Result ESPNtp::NtpOnSyncSetCallback(std::function<void(DateTime newTime)> callback)
{
    ContextLock lock(mutex);
    RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));
    ESPNtp::callback = callback;
    return Result();
}


