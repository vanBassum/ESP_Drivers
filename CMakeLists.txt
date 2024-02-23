#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

idf_component_register(
	SRCS
		esp_drivers.cpp
		analog/ina3221.cpp
		displays/ST7796S.cpp
		ethernet/lan87xx.cpp
		ethernet/TCPClient.cpp
		ethernet/wifi.cpp
		expanders/MAX14830.cpp
		expanders/MAX14830Uart.cpp
		expanders/mcp23s17.cpp
		gpio/espgpio.cpp
		i2c/bus.cpp
		i2c/device.cpp
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
		gpio
		i2c
		interfaces
		onewire
		spi
	REQUIRES ESP_Base driver esp_wifi esp_eth)














