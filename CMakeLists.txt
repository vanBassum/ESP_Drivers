#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

idf_component_register(
	SRCS
		"esp_drivers.cpp"
		"display/hd44780.cpp"
		"display/t6963c.cpp"
		"display/st7735.cpp"
		"expander/max14830.cpp"
		"expander/mcp23s17.cpp"
		"rtc/pcf2123.cpp"
		"mcu/esp32.cpp"
		"measure/ina3221.cpp"
	INCLUDE_DIRS "." "display" "expander" "rtc" "mcu" "measure"
	REQUIRES ESP_Base ESP_Com
	PRIV_REQUIRES esp_wifi)
