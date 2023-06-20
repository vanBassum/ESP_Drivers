#include "lan87xx.h"

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

bool LAN87xx::Init(NetManager* netManager)
{
    EventGroupHandle_t eth_event_group = xEventGroupCreate();
    
    INIT_OR_RETURN(TAG, "eth event group", eth_event_group != NULL);

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();  // apply default MAC configuration
    eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    esp_eth_mac_t* mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config); // create MAC instance
   // TEST_ASSERT_NOT_NULL(mac);
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG(); // apply default PHY configuration
    esp_eth_phy_t* phy = esp_eth_phy_new_lan87xx(&phy_config);

    //TEST_ASSERT_NOT_NULL(phy);
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy); // apply default driver configuration
    esp_eth_handle_t eth_handle = NULL; // after driver installed, we will get the handle of the driver
    INIT_OR_RETURN(TAG, "eth_driver", esp_eth_driver_install(&config, &eth_handle) == ESP_OK); // install driver
    //TEST_ASSERT_NOT_NULL(eth_handle);
    INIT_OR_RETURN(TAG, "eth_event_loop", esp_event_loop_create_default() == ESP_OK);
    //TEST_ESP_OK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, eth_event_group));
    INIT_OR_RETURN(TAG, "eth_start", esp_eth_start(eth_handle) == ESP_OK); // start Ethernet driver state machine

	//eth_netif = esp_netif_create_default_eth();
	INIT_OR_RETURN(TAG, "Create Eth", eth_netif != NULL);
	return false;	
};

