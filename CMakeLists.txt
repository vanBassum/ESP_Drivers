#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

idf_component_register(
	SRCS
		esp_drivers.cpp
		analog/ina3221.cpp
		ethernet/lan87xx.cpp
		ethernet/wifi.cpp
		i2c/bus.cpp
		i2c/device.cpp
		spi/bus.cpp
		spi/device.cpp
	INCLUDE_DIRS
		.
		analog
		common
		devicemanagement
		ethernet
		flash
		i2c
		interfaces
		kernel
		onewire
		spi
	REQUIRES ESP_Base driver esp_wifi esp_eth)














