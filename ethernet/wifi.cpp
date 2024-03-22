#include "wifi.h"
#include <sdkconfig.h>
#include "esp_event.h"

static const char *TAG	= "Wifi";

void  Wifi::Wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		ESP_LOGW(TAG, "is Disconnected retry");  //Retry connecting.
		esp_wifi_connect();			
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ESP_LOGI(TAG, "Wifi IP:" IPSTR, IP2STR(&ip_info->ip));
        ESP_LOGI(TAG, "Connected");  
	}
}

void Wifi::got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Wifi Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "Wifi IP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "Wifi MASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "Wifi GW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}

Result Wifi::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);				//Protect public functions with a mutex!
	
	RETURN_ON_ERR(config.getProperty("NetIF", &netIfDeviceKey));
    	
	// optional property
	//config.getProperty("mySetting", &mySetting);
	
	//Tell the devicemanager the driver is ready to load dependecies (Use early return in case of problems)
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result Wifi::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);				//Protect public functions with a mutex!
	
	//RETURN_ON_ERR(deviceManager->getDeviceByKey<NetIF>(netIfDeviceKey, netIfDevice));
	
	//Tell the devicemanager the driver is ready for initialisation. Since a dependency can be ready now, but can break at any time, it makes no sense to check the dependency for ready state, we do this later, 
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result Wifi::DeviceInit()
{ esp_err_t err = 0;
	//esp_netif_t *wifi_netif;
	wifi_init_config_t cfg2 = WIFI_INIT_CONFIG_DEFAULT();
	
	wifi_netif = esp_netif_create_default_wifi_sta();
	if(wifi_netif == NULL) return Result::Error;
	err = esp_wifi_init(&cfg2);
	if(err != ESP_OK) return Result::Error;
	err = esp_wifi_set_storage(WIFI_STORAGE_RAM); 
	if(err != ESP_OK) return Result::Error;

    DeviceSetStatus(DeviceStatus::Ready);   
    return Result::Ok;
}

Result Wifi::Connect(const std::string& ssid, const std::string& pwd)
{ esp_err_t err = 0;
	wifi_config_t wifi_config = { };
	memcpy(wifi_config.sta.ssid, ssid.c_str(), ssid.size());
	memcpy(wifi_config.sta.password, pwd.c_str(), pwd.size());
	ESP_LOGI(TAG, "Connecting to SSID %s", ssid.c_str());  
	ESP_LOGI(TAG, "Using Password %s", pwd.c_str());
	
	err = esp_wifi_set_mode(WIFI_MODE_STA);
    if(err != ESP_OK) return Result::Error;

	err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
	if(err != ESP_OK) return Result::Error;

	err = esp_wifi_start();
	if(err != ESP_OK) return Result::Error;

	err = esp_wifi_connect();
	if(err != ESP_OK) return Result::Error;

	esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi_event_handler, NULL, &instance_any_id));
	// We can choose to use a combined event handler or two separate handlers.... for now use the combined.
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Wifi_event_handler, NULL, &instance_got_ip));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip_event_handler, NULL, &instance_got_ip));
	DeviceSetStatus(DeviceStatus::Ready);   
    return Result::Ok;
}

Result Wifi::GetIpInfo(esp_netif_ip_info_t* ip_info)
{
	esp_netif_get_ip_info(wifi_netif, ip_info);
    return Result::Ok;
}
	
Result Wifi::SetDNS(esp_ip4_addr_t ip, esp_netif_dns_type_t type)
{
	esp_netif_dns_info_t dns;
	dns.ip.u_addr.ip4.addr = ip.addr;
	dns.ip.type = IPADDR_TYPE_V4;
	ESP_ERROR_CHECK(esp_netif_set_dns_info(wifi_netif, type, &dns));
    return Result::Ok;
}	
		
Result Wifi::SetStaticIp(esp_netif_ip_info_t ip)
{
	if (esp_netif_dhcpc_stop(wifi_netif) != ESP_OK) {
		ESP_LOGE(TAG, "Failed to stop dhcp client");
		return Result::Error;
	}
		if (esp_netif_set_ip_info(wifi_netif, &ip) != ESP_OK) {
		ESP_LOGE(TAG, "Failed to set ip info");
		return Result::Error;
	}
	ESP_LOGD(TAG, "Success to set static ip " IPSTR " " IPSTR " " IPSTR " ", IP2STR(&ip.ip), IP2STR(&ip.gw), IP2STR(&ip.netmask));
    return Result::Ok;
}
	
Result Wifi::SetDHCP()
{
	if (esp_netif_dhcpc_start(wifi_netif) != ESP_OK) {
		ESP_LOGE(TAG, "Failed to start dhcp client");
		return Result::Error;
	}
    return Result::Ok;
}
