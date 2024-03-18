#include "lan87xx.h"
#include "esp_eth.h"
#include <esp_eth_mac.h>
#include <sdkconfig.h>
#include "esp_event.h"
#include <freertos\event_groups.h>

#define ETH_PHY_RST_GPIO 	-1
#define ETH_MDC_GPIO 		23
#define ETH_MDIO_GPIO		18
#define CONFIG_EXAMPLE_ETH_PHY_ADDR		0

#define ETH_START_BIT BIT(0)
#define ETH_STOP_BIT BIT(1)
#define ETH_CONNECT_BIT BIT(2)

void LAN87xx::eth_event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data) {
/*     EventGroupHandle_t eth_event_group = (EventGroupHandle_t)arg;
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;
    uint8_t mac_addr[6] = {0};

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        //xEventGroupSetBits(eth_event_group, ETH_CONNECT_BIT);
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        ESP_LOGI("ETH", "Connected");  
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI("ETH", "Disconnected");  
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        //xEventGroupSetBits(eth_event_group, ETH_START_BIT);
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        //xEventGroupSetBits(eth_event_group, ETH_STOP_BIT);
        break;
    default:
        break;
    } */
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
void LAN87xx::got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    // ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    // const esp_netif_ip_info_t *ip_info = &event->ip_info;

    // ESP_LOGI(TAG, "Ethernet Got IP Address");
    // ESP_LOGI(TAG, "~~~~~~~~~~~");
    // ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    // ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    // ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    // ESP_LOGI(TAG, "~~~~~~~~~~~");
}

Result LAN87xx::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);				//Protect public functions with a mutex!
	
	RETURN_ON_ERR(config.getProperty("NetIF", &netIfDeviceKey));
    	
	// optional property
	//config.getProperty("mySetting", &mySetting);
	
	//Tell the devicemanager the driver is ready to load dependecies (Use early return in case of problems)
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result LAN87xx::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);				//Protect public functions with a mutex!
	
	//RETURN_ON_ERR(deviceManager->getDeviceByKey<NetIF>(netIfDeviceKey, netIfDevice));
	
	//Tell the devicemanager the driver is ready for initialisation. Since a dependency can be ready now, but can break at any time, it makes no sense to check the dependency for ready state, we do this later, 
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result LAN87xx::DeviceInit(IDeviceConfig &config)
{ esp_err_t err = 0;
    // Init MAC and PHY configs to default
   /*  eth_mac_config_t mac_config                 = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config                 = ETH_PHY_DEFAULT_CONFIG();

    // Update PHY config based on board specific configuration
    phy_config.phy_addr                         = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num                   = ETH_PHY_RST_GPIO;

    // Init vendor specific MAC config to default
    eth_esp32_emac_config_t esp32_emac_config   = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    esp32_emac_config.smi_mdc_gpio_num          = ETH_MDC_GPIO;
    esp32_emac_config.smi_mdio_gpio_num         = ETH_MDIO_GPIO;

    // Create new ESP32 Ethernet MAC instance
    esp_eth_mac_t* mac                          = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);
    if(mac == NULL) return Result::Error;
    esp_eth_phy_t* phy                          = esp_eth_phy_new_lan87xx(&phy_config);
    if(phy == NULL) return Result::Error;
    // Install driver
    esp_eth_handle_t eth_handle                 = NULL;
    esp_eth_config_t eth_config                 = ETH_DEFAULT_CONFIG(mac, phy);
	err = esp_eth_driver_install(&eth_config, &eth_handle);
    if(err != ESP_OK) return Result::Error;
    // Create new default instance of esp-netif for Ethernet
    esp_netif_config_t cfg                      = ESP_NETIF_DEFAULT_ETH();
	esp_netif_t *eth_netif                         = esp_netif_new(&cfg);

	err = esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle));
    if(err != ESP_OK) return Result::Error;

    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

	err = esp_eth_start(eth_handle);
    if(err != ESP_OK) return Result::Error; */

    DeviceSetStatus(DeviceStatus::Ready);   
    return Result::Ok;
}

