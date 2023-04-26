#pragma once

#include "spi/bus.h"
#include "kernel.h"
#include "spi.h"
#include "driver/gpio.h"
#include "interfaces.h"
#include "esp_base.h"

//https://www.maximintegrated.com/en/products/interface/controllers-expanders/MAX14830.html



#define MAX14830_BRGCFG_CLKDIS_BIT		(1 << 6) /* Clock Disable */
#define MAX14830_REV_ID					(0xb0)
#define MAX14830_USE_XTAL
#define MAX14830_CLK					4000000
#define MAX14830_FIFO_MAX				128


	
class MAX14830 : public IGPIO
{
	enum max14830_uart_port_t 
	{
		MAX14830_UART_NUM_0 = 0x0,
		MAX14830_UART_NUM_1 = 0x1,
		MAX14830_UART_NUM_2 = 0x2,
		MAX14830_UART_NUM_3 = 0x3,
	};

	enum max14830_pinmodes_t
	{
		MAX14830_PINMODE_INPUT,
		MAX14830_PINMODE_PUSHPULL,
		MAX14830_PINMODE_OPENDRAIN
	};
		
	enum max14830_events_t
	{
		MAX14830_EVENT_NONE     = 0,
		MAX14830_EVENT_IRQ      = (1 << 0),
		MAX14830_EVENT_PORT0_TX = (1 << 1),
		MAX14830_EVENT_PORT1_TX = (1 << 2),
		MAX14830_EVENT_PORT2_TX = (1 << 3),
		MAX14830_EVENT_PORT3_TX = (1 << 4),
	};
		
	bool clockErr = false;
	uint8_t gpioConfBuffer[4];
	uint8_t gpioDataBuffer[4];
	uint8_t gpioIRQBuffer[4];
	SPI::Device spidev;
	Task irqTask;
	gpio_num_t irqPin = GPIO_NUM_NC;
	
	static void IRAM_ATTR gpio_isr_handler(void* arg);
		
public:
	class Uart : public IStream
	{
		MAX14830* parent;
		max14830_uart_port_t port;
		StreamBuffer inputBuffer;
		StreamBuffer outputBuffer;
		void OnDataReady(IStream* buffer);
	protected:
		void HandleIRQ(uint32_t* changes);
		void HandleOutputBuffer();
		friend MAX14830;
	public:
		Uart(MAX14830* parent, max14830_uart_port_t port);
		void Init(uint32_t baudrate, uint8_t useCTS, uint8_t useRS485);
		size_t Write(const void* data, size_t size) override;
		size_t Read(void* data, size_t size) override;
	};
	
	
protected:
	void IrqTaskWork(Task* task, void* args);
	esp_err_t Detect();
	esp_err_t SetRefClock();
	esp_err_t Max14830_WriteBufferPolled(uint8_t cmd, const uint8_t * cmdData, uint8_t count);
	esp_err_t Max14830_ReadBufferPolled(uint8_t cmd, uint8_t * cmdData, uint8_t * replyData, uint8_t count);
	esp_err_t regmap_write(uint8_t cmd, uint8_t value);
	esp_err_t regmap_read(uint8_t cmd, uint8_t * value);
	uint8_t max310x_port_read(max14830_uart_port_t port, uint8_t cmd);
	void max310x_port_write(max14830_uart_port_t port, uint8_t cmd, uint8_t value);
	void max310x_port_update(max14830_uart_port_t port, uint8_t cmd, uint8_t mask, uint8_t value);
	uint8_t max310x_update_best_err(uint64_t f, int64_t *besterr);
	uint32_t max310x_set_ref_clk();
	uint32_t max310x_set_baud(max14830_uart_port_t port, uint32_t baud);
	uint32_t max310x_get_ref_clk();
	
	friend Uart;
public:
	Uart Uart0 = Uart(this, MAX14830_UART_NUM_0);
	Uart Uart1 = Uart(this, MAX14830_UART_NUM_1);
	Uart Uart2 = Uart(this, MAX14830_UART_NUM_2);
	Uart Uart3 = Uart(this, MAX14830_UART_NUM_3);

	bool Init(SPI::Bus* spiBus, gpio_num_t cs, gpio_num_t irq, transaction_cb_t pre_cb  = NULL, transaction_cb_t post_cb = NULL);	
	virtual uint32_t ReadPins(uint32_t bank, uint32_t mask) override;
	virtual void SetMode(uint32_t bank, uint32_t mask, Mode mode) override;
	virtual void WritePins(uint32_t bank, uint32_t mask, uint32_t value) override;
};
