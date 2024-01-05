#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

idf_component_register(
	SRCS
		esp_drivers.cpp
		analog/ina3221.cpp
		displays/hd44780.cpp
		displays/st7735.cpp
		displays/st7796s.cpp
		displays/t6963c.cpp
		ethernet/lan87xx.cpp
		ethernet/wifi.cpp
		expanders/max14830.cpp
		expanders/mcp23s17.cpp
		i2c/bus.cpp
		i2c/device.cpp
		mifare/pn512.cpp
		rtc/ntp.cpp
		rtc/pcf2123.cpp
		spi/bus.cpp
		spi/device.cpp
	INCLUDE_DIRS
		.
		analog
		common
		displays
		ethernet
		expanders
		flash
		i2c
		kernel
		mifare
		onewire
		rtc
		spi
	REQUIRES ESP_Base driver esp_wifi esp_eth)














