#pragma once


class NetIF
{
	const char* TAG = "NetIF";
protected:
	esp_netif_t *netIF;
	
public:
	
	NetIF()
	{
		
	}
	
		
	void GetIpInfo(esp_netif_ip_info_t* ip_info)
	{
		esp_netif_get_ip_info(netIF, ip_info);
	}
	
	void SetDNS(esp_ip4_addr_t ip, esp_netif_dns_type_t type)
	{
		esp_netif_dns_info_t dns;
		dns.ip.u_addr.ip4.addr = ip.addr;
		dns.ip.type = IPADDR_TYPE_V4;
		ESP_ERROR_CHECK(esp_netif_set_dns_info(netIF, type, &dns));
	}
	
		
	void SetStaticIp(esp_netif_ip_info_t ip)
	{
		if (esp_netif_dhcpc_stop(netIF) != ESP_OK) {
			ESP_LOGE(TAG, "Failed to stop dhcp client");
			return;
		}

		if (esp_netif_set_ip_info(netIF, &ip) != ESP_OK) {
			ESP_LOGE(TAG, "Failed to set ip info");
			return;
		}
		ESP_LOGD(TAG, "Success to set static ip " IPSTR " " IPSTR " " IPSTR " ", IP2STR(&ip.ip), IP2STR(&ip.gw), IP2STR(&ip.netmask));
	}
	
	void SetDHCP()
	{
		if (esp_netif_dhcpc_start(netIF) != ESP_OK) {
			ESP_LOGE(TAG, "Failed to start dhcp client");
			return;
		}
	}
	
	
};


