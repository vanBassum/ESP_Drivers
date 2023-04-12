#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

idf_component_register(
	SRCS
		"esp_drivers.cpp"
		"analog/ina3221.cpp"
		"displays/hd44780.cpp"
		"rtc/pcf2123.cpp"
		"expanders/mcp23s17.cpp"
		"displays/st7735.cpp"
		"spi/bus.cpp"
		"spi/device.cpp"
		"i2c/device.cpp"
		"i2c/bus.cpp"
		"expanders/max14830.cpp"
		"displays/t6963c.cpp"
		"ethernet/socket.cpp"
		"netif/wifi.cpp"
	INCLUDE_DIRS
		"."
		"analog"
		"kernel"
		"rtc"
		"spi"
		"displays"
		"ethernet"
		"expanders"
		"flash"
		"i2c"
		"interfaces"
		"netif"
	REQUIRES ESP_Base driver
	PRIV_REQUIRES esp_wifi)














