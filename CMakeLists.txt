#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

idf_component_register(
	SRCS
		"esp_drivers.cpp"
		"displays\hd44780.cpp"
		"rtc\pcf2123.cpp"
		"expanders\mcp23s17.cpp"
		"displays\st7735.cpp"
		"measure\ina3221.cpp"
		"mcu\esp32.cpp"
		"spi\bus.cpp"
		"syslog\syslog.cpp"
		"eth\socket.cpp"
		"spi\device.cpp"
		"i2c\device.cpp"
		"i2c\bus.cpp"
		"expanders\max14830.cpp"
		"displays\t6963c.cpp"
	INCLUDE_DIRS 
		"." 
		"kernel"
		"mcu"	
		"measure"	
		"rtc"	
		"spi"	
		"syslog"	
		"displays"	
		"eth"	
		"expanders"	
		"flash"	
		"i2c"	
		"interfaces"	
	REQUIRES 
		ESP_Base
	PRIV_REQUIRES 
		esp_wifi)














