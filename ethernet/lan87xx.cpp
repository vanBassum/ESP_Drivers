#include "lan87xx.h"
#include "esp_eth.h"
#include <esp_eth_mac.h>
#include <sdkconfig.h>

#define ETH_PHY_RST_GPIO 	-1
#define ETH_MDC_GPIO 		23
#define ETH_MDIO_GPIO		18
#define CONFIG_EXAMPLE_ETH_PHY_ADDR		0
//#define CONFIG_ETH_USE_ESP32_EMAC

/*
 #define ETH_START_BIT BIT(0)
#define ETH_STOP_BIT BIT(1)
#define ETH_CONNECT_BIT BIT(2)

static void eth_event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data) {
    EventGroupHandle_t eth_event_group = (EventGroupHandle_t)arg;
    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        xEventGroupSetBits(eth_event_group, ETH_CONNECT_BIT);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        break;
    case ETHERNET_EVENT_START:
        xEventGroupSetBits(eth_event_group, ETH_START_BIT);
        break;
    case ETHERNET_EVENT_STOP:
        xEventGroupSetBits(eth_event_group, ETH_STOP_BIT);
        break;
    default:
        break;
    }
}*/

Result LAN87xx::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);				//Protect public functions with a mutex!
	
	RETURN_ON_ERR(config.getProperty("netifDevice", &netIfDeviceKey));
    	
	// optional property
	//config.getProperty("mySetting", &mySetting);
	
	//Tell the devicemanager the driver is ready to load dependecies (Use early return in case of problems)
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result LAN87xx::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);				//Protect public functions with a mutex!
	
	RETURN_ON_ERR(deviceManager->getDeviceByKey<NetIF>(netIfDeviceKey, netIfDevice));
	
	//Tell the devicemanager the driver is ready for initialisation. Since a dependency can be ready now, but can break at any time, it makes no sense to check the dependency for ready state, we do this later, 
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result LAN87xx::DeviceInit(IDeviceConfig &config)
{ esp_err_t err = 0;
    // Init MAC and PHY configs to default
    eth_mac_config_t mac_config                 = ETH_MAC_DEFAULT_CONFIG();
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
    esp_eth_phy_t* phy                          = esp_eth_phy_new_lan87xx(&phy_config);

    // Install driver
    esp_eth_handle_t eth_handle                 = NULL;
    esp_eth_config_t eth_config                 = ETH_DEFAULT_CONFIG(mac, phy);
	err = esp_eth_driver_install(&eth_config, &eth_handle);
    if(err != ESP_OK) return Result::Error;
    // Create new default instance of esp-netif for Ethernet
    esp_netif_config_t cfg                      = ESP_NETIF_DEFAULT_ETH();
	esp_netif_t *netIF                          = esp_netif_new(&cfg);

	err = esp_netif_attach(netIF, esp_eth_new_netif_glue(eth_handle));
    if(err != ESP_OK) return Result::Error;
	err = esp_eth_start(eth_handle);
    if(err != ESP_OK) return Result::Error;

    return Result::Ok;
}

