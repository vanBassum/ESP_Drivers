#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

idf_component_register(
	SRCS
		esp_drivers.cpp
		analog/ina3221.cpp
		displays/hd44780.cpp
		displays/ST7796S.cpp
		esp/ESPNtp.cpp
		esp/Uart.cpp
		ethernet/ESPWifi.cpp
		ethernet/lan87xx.cpp
		ethernet/netif.cpp
		ethernet/TCPClient.cpp
		expanders/MAX14830.cpp
		expanders/MAX14830Uart.cpp
		expanders/mcp23s17.cpp
		gpio/espgpio.cpp
		i2c/bus.cpp
		i2c/device.cpp
		rtc/pcf2123.cpp
		spi/bus.cpp
		spi/device.cpp
	INCLUDE_DIRS
		.
		analog
		common
		displays
		esp
		ethernet
		expanders
		flash
		gpio
		i2c
		interfaces
		onewire
		rtc
		spi
	REQUIRES ESP_Base driver esp_wifi esp_eth)














