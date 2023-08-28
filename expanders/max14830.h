#pragma once

#include "spi/bus.h"
#include "kernel.h"
#include "spi.h"
#include "driver/gpio.h"
#include "esp_base.h"

//https://www.maximintegrated.com/en/products/interface/controllers-expanders/MAX14830.html



#define MAX14830_BRGCFG_CLKDIS_BIT		(1 << 6) /* Clock Disable */
#define MAX14830_REV_ID					(0xb0)
#define MAX14830_USE_XTAL
#define MAX14830_CLK					4000000
#define MAX14830_FIFO_MAX				128

class MAX14830
{
	const char* TAG = "MAX14830";
	bool clockErr = false;
	uint8_t gpioConfBuffer[4];
	uint8_t gpioDataBuffer[4];
	uint8_t gpioIRQBuffer[4];
	SPIDevice& spidev;
	Task irqTask;
	gpio_num_t irqPin = GPIO_NUM_NC;
	
	static void IRAM_ATTR gpio_isr_handler(void* arg);
		
public:
	
	
	enum class Ports
	{
		NUM_0 = 0x0,
		NUM_1 = 0x1,
		NUM_2 = 0x2,
		NUM_3 = 0x3,
	};
	
	enum class PinModes
	{
		INPUT,
		PUSHPULL,
		OPENDRAIN
	};

	enum class Pins
	{
		NONE = 0,
		D0    = (1 << 0),
		D1    = (1 << 1),
		D2    = (1 << 2),
		D3    = (1 << 3),
		D4    = (1 << 4),
		D5    = (1 << 5),
		D6    = (1 << 6),
		D7    = (1 << 7),
		D8    = (1 << 8),
		D9    = (1 << 9),
		D10   = (1 << 10),
		D11   = (1 << 11),
		D12   = (1 << 12),
		D13   = (1 << 13),
		D14   = (1 << 14),
		D15   = (1 << 15),
		ALL  = 0xFFFF,
	};
	

	enum class Events
	{
		NONE	= 0,
		IRQ		= (1 << 0),
	};

	class Uart
	{
		const char* TAG = "MAX14830::UART";
		MAX14830& parent;
		Ports port;
		void NotifyTxAvailable();
		friend MAX14830;
		
	public:
		Event<Uart&> DataReceived;
		Uart(MAX14830& parent, Ports port);
		void Init(uint32_t baudrate, uint8_t useCTS, uint8_t useRS485);
		size_t Write(const void* data, size_t size);
		size_t Read(void* data, size_t size);
	};
	
	
protected:
	void IrqTaskWork(Task* task, void* args);
	bool Detect();
	esp_err_t SetRefClock();
	void Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t * cmdData, uint8_t count);
	void Max14830_ReadBufferPolled(uint8_t cmd, uint8_t * cmdData, uint8_t * replyData, uint8_t count);
	void regmap_write(uint8_t cmd, uint8_t value);
	void regmap_read(uint8_t cmd, uint8_t * value);
	uint8_t max310x_port_read(Ports port, uint8_t cmd);
	void max310x_port_write(Ports port, uint8_t cmd, uint8_t value);
	void max310x_port_update(Ports port, uint8_t cmd, uint8_t mask, uint8_t value);
	uint8_t max310x_update_best_err(uint64_t f, int64_t *besterr);
	uint32_t max310x_set_ref_clk();
	uint32_t max310x_set_baud(Ports port, uint32_t baud);
	uint32_t max310x_get_ref_clk();
	void CheckForPinChanges(Ports port, Pins* changes, bool* uartTX);
	friend Uart;
public:
	Event<MAX14830*, Pins> OnPinsChanged;
	MAX14830(SPIDevice& device, gpio_num_t irq);
	void SetPinsMode(Pins mask, PinModes mode);
	void SetPins(Pins mask, Pins value);
	void SetInterrupts(Pins mask, Pins value);
	Pins GetPins(Pins mask);
	Uart Uart0 = Uart(*this, Ports::NUM_0);
	Uart Uart1 = Uart(*this, Ports::NUM_1);
	Uart Uart2 = Uart(*this, Ports::NUM_2);
	Uart Uart3 = Uart(*this, Ports::NUM_3);
	
};

DEFINE_ENUM_CLASS_FLAG_OPERATORS(MAX14830::Pins,	uint32_t);
DEFINE_ENUM_CLASS_FLAG_OPERATORS(MAX14830::Events,	uint32_t);


