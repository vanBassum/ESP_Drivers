#include "lan87xx.h"
#include "sdkconfig.h"

#define ETH_PHY_RST_GPIO 	-1
#define ETH_MDC_GPIO 		23
#define ETH_MDIO_GPIO		18
#define CONFIG_EXAMPLE_ETH_PHY_ADDR		0

static const char *TAG = "Lan87xx";
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

Lan87xx::Lan87xx(NetManager& netManager)
{
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
    esp_eth_config_t config                     = ETH_DEFAULT_CONFIG(mac, phy);
	ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));

    // Create new default instance of esp-netif for Ethernet
    esp_netif_config_t cfg                      = ESP_NETIF_DEFAULT_ETH();
    eth_netif                                   = esp_netif_new(&cfg);

	ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
	ESP_ERROR_CHECK(esp_eth_start(eth_handle));

};

