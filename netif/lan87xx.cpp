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

bool Lan87xx::Init(NetManager* netManager)
{
    
    // Initialize TCP/IP network interface (should be called only once in application)
    INIT_OR_RETURN(TAG, "netif_init", esp_netif_init() == ESP_OK);
    // Create default event loop that running in background
    INIT_OR_RETURN(TAG, "loop_default", esp_event_loop_create_default() == ESP_OK);

    // Create new default instance of esp-netif for Ethernet
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    //esp_netif_t* eth_netif = esp_netif_new(&cfg);

    // Init MAC and PHY configs to default
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = ETH_PHY_RST_GPIO;
    eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    esp32_emac_config.smi_mdc_gpio_num = ETH_MDC_GPIO;
    esp32_emac_config.smi_mdio_gpio_num = ETH_MDIO_GPIO;
    esp_eth_mac_t* mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);

    esp_eth_phy_t* phy = esp_eth_phy_new_lan87xx(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    //ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    INIT_OR_RETURN(TAG, "eth_driver", esp_eth_driver_install(&config, &eth_handle) == ESP_OK); // install driver
    /* attach Ethernet driver to TCP/IP stack */
    //ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    INIT_OR_RETURN(TAG, "eth_driver", esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)) == ESP_OK);
    // Register user defined event handers
 //   ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
 //   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    /* start Ethernet driver state machine */
    //ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    INIT_OR_RETURN(TAG, "eth_state_machine", (esp_eth_start(eth_handle)) == ESP_OK);
	return true;	
};

