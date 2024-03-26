/*
* The MIT License (MIT)
*
* Copyright (c) 2016 Jaakko Salo (jaakkos@gmail.com / jaakkos on Freenode)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#pragma once

/* https://github.com/PaulStoffregen/Time */
//https://github.com/torvalds/linux/blob/master/drivers/rtc/rtc-pcf2123.c
//#include "kernel.h"
//#include "mtime.h"
//#include "spi.h"
//#include "driver/gpio.h"
#include <memory>
#include "spi/device.h"
#include "IRtc.h"

struct PCF2123_CtrlRegs
{
	/**
		* Control register bits. Default values marked with asterisk.
		* Enum values are bit positions into the registers.
		*/

	enum Ctrl1Regs {
		EXT_TEST = 7,
		/* 0*: normal mode, 1: external clock test mode */
		STOP = 5,
		/* 0*: RTC clock runs, 1: RTC clock stopped */
		SR = 4,
		/* 0*: no software reset, 1: initiate software reset */
		HOUR_MODE = 2,
		/* 0*: 24h mode, 1: 12h mode. We only use 24h mode */
		CIE = 1   /* 0*: no correction interrupt generated, 1: pulse
			                generated at every correction cycle */
	};

	enum Ctrl2Regs {
		MI = 7 + 8,
		/* 0*: minute int. disabled, 1: minute int. enabled */
		SI = 6 + 8,
		/* 0*: second int. disabled, 1: second int. enabled */
		MSF = 5 + 8,
		/* 0*: no m/s int. generated, 1: int. has been generated */
		TI_TP = 4 + 8,
		/* 0*: int. follows timer flags, 1: int. generates pulse */
		AF = 3 + 8,
		/* 0*: no alarm int. generated, 1: alarm triggered */
		TF = 2 + 8,
		/* 0*: no countdown int. generated, 1: countdown triggd */
		AIE = 1 + 8,
		/* 0*: don't generate alarm int., 1: generate alarm int. */
		TIE = 0 + 8   /* 0*: don't generate countdown int., 1: generate countdown
			                int. */
	};

	uint8_t ctrl[2]; /**< Control registers 1 and 2 */
	bool get(int bit);
	bool set(int bit, bool value);
	void mask_alarms(void);
	
	void ClearAll()
	{
		ctrl[0] = 0;
		ctrl[1] = 0;
	}
};

class PCF2123 : public IRtc
{
 	constexpr static const char* TAG = "PCF2123";
	Mutex mutex;

	//config
    const char *spiDeviceKey = nullptr;
	std::shared_ptr<SpiDevice> spiDevice;

public:
	enum CountdownSrcClock {
		CNTDOWN_CLOCK_4096HZ   = 0,
		CNTDOWN_CLOCK_64HZ     = 1,
		CNTDOWN_CLOCK_1HZ      = 2,
		CNTDOWN_CLOCK_1PER60HZ = 3
	};

	virtual Result DeviceSetConfig(IDeviceConfig& config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

	Result TimeGet(DateTime& value);
	Result TimeSet(DateTime& value);
private:

	enum RxtMode { RXT_READ, RXT_WRITE } ;

	gpio_num_t irq = GPIO_NUM_NC;
	Result reset(void);
	void mask_alarms(void);
	Result rxt(uint8_t addr, uint8_t rw, uint8_t *buf, size_t sz);
	uint8_t bcd_decode(uint8_t bcd);
	uint8_t bcd_encode(uint8_t dec);
	Result ctrl_get(PCF2123_CtrlRegs *regs);
	Result ctrl_set(PCF2123_CtrlRegs *regs, bool set_ctrl1, bool set_ctrl2, bool mask_alarms);
};




