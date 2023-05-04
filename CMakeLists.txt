#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

idf_component_register(
	SRCS
		esp_drivers.cpp
		analog/ina3221.cpp
		displays/hd44780.cpp
		displays/st7735.cpp
		displays/t6963c.cpp
		ethernet/socket.cpp
		expanders/max14830.cpp
		expanders/mcp23s17.cpp
		i2c/bus.cpp
		i2c/device.cpp
		netif/lan8742a.cpp
		netif/netmanager.cpp
		netif/wifi.cpp
		rtc/ntp.cpp
		rtc/pcf2123.cpp
		spi/bus.cpp
		spi/device.cpp
	INCLUDE_DIRS
		.
		analog
		displays
		ethernet
		expanders
		flash
		i2c
		interfaces
		kernel
		netif
		rtc
		spi
	REQUIRES ESP_Base driver esp_wifi)














