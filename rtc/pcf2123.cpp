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

#include "datetime.h"
#include "kernel.h"
#include "pcf2123.h"
#include "esp_log.h"
#include "ISpiDevice.h"
#include "spi/device.h"

#define SPI_MAX_SPEED			  5000000

#define REG_CTRL1_ADDR            0x00
#define REG_CTRL2_ADDR            0x01
#define REG_TIME_DATE_ADDR        0x02
#define REG_ALARM_ADDR            0x09
#define REG_OFFSET_ADDR           0x0D
#define REG_TIMER_CLKOUT_ADDR     0x0E
#define REG_COUNTDOWN_TIMER_ADDR  0x0F


bool
PCF2123_CtrlRegs::get(int bit)
{
	return this->ctrl[bit / 8] & (1 << (bit % 8));
}

bool
PCF2123_CtrlRegs::set(int bit, bool value)
{
	bool old = this->get(bit);

	if (value) this->ctrl[bit / 8] |=  (1 << (bit % 8));
	else       this->ctrl[bit / 8] &= ~(1 << (bit % 8));

	return old;
}

void PCF2123_CtrlRegs::mask_alarms(void)
{
	this->set(MSF, true);
	this->set(AF, true);
	this->set(TF, true);

//	return DeviceResult::Ok;
}

DeviceResult PCF2123::rxt(uint8_t addr, uint8_t rw, uint8_t *buf, size_t sz)
{
	if (sz < 1)
		return;

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       				
	t.length = (sz * 8);       
	t.tx_buffer = buf; 
	t.rx_buffer = buf;               			
	t.cmd = 0x10 | (rw == RXT_READ ? 0x80 : 0x00) | addr;								
	//spidev->PollingTransmit(&t);  	
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(
		spiDevice->Transmit(&t, SPIFlags::POLLED ) == DeviceResult::Ok,
		 DeviceStatus::Dependencies, DeviceResult::Error, TAG, "spiDevice->Transmit returned error");

	return DeviceResult::Ok;		
}


uint8_t
PCF2123::bcd_decode(uint8_t bcd)
{
	return (bcd >> 4) * 10 + (bcd & 0x0F);
}

uint8_t
PCF2123::bcd_encode(uint8_t dec)
{
	return ((dec / 10) << 4) | (dec % 10);
}

DeviceResult PCF2123::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiDevice", &spiDeviceKey),  DeviceStatus::ConfigError, DeviceResult::Error, TAG, "Missing parameter: spiDevice");
	DeviceSetStatus(DeviceStatus::Dependencies);
	return DeviceResult::Ok;
}

DeviceResult PCF2123::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	GET_DEV_OR_RETURN(spiDevice, deviceManager->getDeviceByKey<ISpiDevice>(spiDeviceKey), DeviceResult::Error, TAG, "Dependencies not ready %d", (int)DeviceGetStatus());
	DeviceSetStatus(DeviceStatus::Initializing);
	return DeviceResult::Ok;
}

DeviceResult PCF2123::DeviceInit()
{
	ContextLock lock(mutex);

	//assert(!initialized_ && "Already initialized");
	//assert(spidev->isInitialized());
	//initialized_ = true;
	reset();
	//spidev->AcquireBus();
	// /* Make sure the clock is in 24h mode */
	// PCF2123_CtrlRegs regs = this->ctrl_get();
	// regs.set(PCF2123_CtrlRegs::HOUR_MODE, 0);
	// this->ctrl_set(&regs, true, false, true);
	
	PCF2123_CtrlRegs regs = this->ctrl_get();
	regs.ClearAll();
	this->ctrl_set(&regs, true, true, false);
	//spidev->ReleaseBus();

	DeviceSetStatus(DeviceStatus::Ready);
	return DeviceResult::Ok;
}


DeviceResult PCF2123::TimeGet(DateTime& now)
{
	//assert(initialized_);
	uint8_t buf[7];
	//spidev->AcquireBus();
	this->rxt(REG_TIME_DATE_ADDR, RXT_READ, buf, sizeof(buf));
	//spidev->ReleaseBus();
	struct tm time;
	memset(&time, 0, sizeof(struct tm));

	time.tm_sec		= bcd_decode(buf[0] & ~0x80);
	time.tm_min		= bcd_decode(buf[1] & ~0x80);
	time.tm_hour	= bcd_decode(buf[2] & ~0xC0); /* 24h clock */
	time.tm_mday    = bcd_decode(buf[3] & ~0xC0);
	time.tm_wday	= bcd_decode(buf[4] & ~0xF8);
	time.tm_mon		= bcd_decode(buf[5] & ~0xE0);
	time.tm_year	= bcd_decode(buf[6]) + 100;		//1900 + 100 + PCF.year value 
	//PCF year = 0 - 99 
	//struct tm year = since 1900
//	now->SetFromUTC(&time); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if ( !(buf[0] & 0x80)) return DeviceResult::Ok;
	else return DeviceResult::Error;
}

DeviceResult PCF2123::TimeSet(DateTime& new_time)
{
	//assert(initialized_);
	uint8_t buf[7];
	struct tm time;
//	new_time->GetAsUTC(&time); !!!!!!!!!!!!!!!!!!!!!!!!!!!

	buf[0] = bcd_encode(time.tm_sec);
	buf[1] = bcd_encode(time.tm_min);
	buf[2] = bcd_encode(time.tm_hour);
	buf[3] = bcd_encode(time.tm_mday);
	buf[4] = bcd_encode(time.tm_wday);
	buf[5] = bcd_encode(time.tm_mon);
	buf[6] = bcd_encode(time.tm_year - 100);	//2023 - 1900 - 100 = 23
	//PCF year = 0 - 99 
	//struct tm year = since 1900
	//spidev->AcquireBus();
	this->rxt(REG_TIME_DATE_ADDR, RXT_WRITE, buf, sizeof(buf));
	//spidev->ReleaseBus();
	return DeviceResult::Ok;
}

DeviceResult PCF2123::reset(void)
{
//	assert(initialized_);
//	spidev->AcquireBus();
	uint8_t buf = 0x58;
	this->rxt(REG_CTRL1_ADDR, RXT_WRITE, &buf, sizeof(buf));
//	spidev->ReleaseBus();
	return DeviceResult::Ok;
}


//void
//PCF2123::stop(bool stopped)
//{
//	PCF2123_CtrlRegs regs = ctrl_get();
//	regs.set(PCF2123_CtrlRegs::STOP, stopped);
//	this->ctrl_set(&regs,
//		true,  /* Write ctrl1 */
//		false, /* No ctrl2 */
//		true   /* Mask alarms */);
//}
//
//bool
//PCF2123::clkout_freq_set(uint16_t freq)
//{
//	uint8_t COF;
//	uint8_t buf;
//
//	switch (freq)
//	{
//	case 0:     COF = 7; break;
//	case 1:     COF = 6; break;
//	case 1024:  COF = 5; break;
//	case 2048:  COF = 4; break;
//	case 4096:  COF = 3; break;
//	case 8192:  COF = 2; break;
//	case 16384: COF = 1; break;
//	case 32768: COF = 0; break;
//	default: return false;
//	}
//
//	this->rxt(REG_TIMER_CLKOUT_ADDR, RXT_READ, &buf, sizeof(buf));
//	buf &= ~0x70;
//	buf |= COF << 4;
//	this->rxt(REG_TIMER_CLKOUT_ADDR, RXT_WRITE, &buf, sizeof(buf));
//	return true;
//}
//
//bool
//PCF2123::countdown_set(bool enable,
//	CountdownSrcClock source_clock,
//	uint8_t value)
//{
//	uint8_t buf[2];
//
//	if (source_clock < 0 || source_clock > 3)
//		return false;
//
//	this->rxt(REG_TIMER_CLKOUT_ADDR, RXT_READ, buf, 2);
//
//	/* First disable the countdown timer. */
//	buf[0] &= ~0x08;
//	this->rxt(REG_TIMER_CLKOUT_ADDR, RXT_WRITE, buf, 2);
//
//	/* Reconfigure timer */
//	buf[0] = (buf[0] & ~0x08) | ((uint8_t)enable << 3);
//	buf[0] = (buf[0] & ~0x03) | source_clock;
//	buf[1] = value;
//	this->rxt(REG_TIMER_CLKOUT_ADDR, RXT_WRITE, buf, 3);
//
//	return true;
//}
//
//uint8_t
//PCF2123::countdown_get()
//{
//	uint8_t buf;
//	this->rxt(REG_COUNTDOWN_TIMER_ADDR, RXT_READ, &buf, sizeof(buf));
//	return buf;
//}
//
//bool
//PCF2123::alarm_set(int minute, int hour, int day, int weekday)
//{
//	uint8_t buf[4];
//
//	if ((minute < 0 || minute > 59) && minute != -1) return false;
//	if ((hour < 0 || hour > 23) && hour != -1) return false;
//	if ((day < 0 || day > 31) && day != -1) return false;
//	if ((weekday < 0 || weekday > 6) && weekday != -1) return false;
//
//	buf[0] = minute < 0 ? 0x80 : bcd_encode(minute);
//	buf[1] = hour < 0 ? 0x80 : bcd_encode(hour);
//	buf[2] = day < 0 ? 0x80 : bcd_encode(day);
//	buf[3] = weekday < 0 ? 0x80 : bcd_encode(weekday);
//	this->rxt(REG_ALARM_ADDR, RXT_WRITE, buf, sizeof(buf));
//
//	return true;
//}

PCF2123_CtrlRegs PCF2123::ctrl_get()
{
	uint8_t buf[2];
	PCF2123_CtrlRegs regs;
	this->rxt(REG_CTRL1_ADDR, RXT_READ, buf, sizeof(buf));
	regs.ctrl[0] = buf[0];
	regs.ctrl[1] = buf[1];
	return regs;
}

void PCF2123::ctrl_set(PCF2123_CtrlRegs *regs,
	bool set_ctrl1,
	bool set_ctrl2,
	bool mask_alarms)
{
	uint8_t buf[2];
	if (mask_alarms)
		regs->mask_alarms();

	if (set_ctrl1 && set_ctrl2)
	{
		buf[0] = regs->ctrl[0];
		buf[1] = regs->ctrl[1];
		this->rxt(REG_CTRL1_ADDR, RXT_WRITE, buf, 2);
	}

	else if (set_ctrl1)
	{
		buf[0] = regs->ctrl[0];
		this->rxt(REG_CTRL1_ADDR, RXT_WRITE, buf, 1);
	}

	else if (set_ctrl2)
	{
		buf[0] = regs->ctrl[1];
		this->rxt(REG_CTRL2_ADDR, RXT_WRITE, buf, 1);
	}
}

