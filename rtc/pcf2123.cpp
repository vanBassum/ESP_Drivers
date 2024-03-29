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
// #include "kernel.h"
#include "pcf2123.h"
#include "esp_log.h"
#include "ISpiDevice.h"
#include "spi/device.h"

#define SPI_MAX_SPEED 5000000

#define REG_CTRL1_ADDR 0x00
#define REG_CTRL2_ADDR 0x01
#define REG_TIME_DATE_ADDR 0x02
#define REG_ALARM_ADDR 0x09
#define REG_OFFSET_ADDR 0x0D
#define REG_TIMER_CLKOUT_ADDR 0x0E
#define REG_COUNTDOWN_TIMER_ADDR 0x0F

bool PCF2123_CtrlRegs::get(int bit)
{
	return this->ctrl[bit / 8] & (1 << (bit % 8));
}

bool PCF2123_CtrlRegs::set(int bit, bool value)
{
	bool old = this->get(bit);

	if (value)
		this->ctrl[bit / 8] |= (1 << (bit % 8));
	else
		this->ctrl[bit / 8] &= ~(1 << (bit % 8));

	return old;
}

void PCF2123_CtrlRegs::mask_alarms(void)
{
	this->set(MSF, true);
	this->set(AF, true);
	this->set(TF, true);
}

Result PCF2123::rxt(uint8_t addr, uint8_t rw, uint8_t *buf, size_t sz)
{
	if (sz < 1)
		return Result::Error;

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = (sz * 8);
	t.tx_buffer = buf;
	t.rx_buffer = buf;
	t.cmd = 0x10 | (rw == RXT_READ ? 0x80 : 0x00) | addr;
	RETURN_ON_ERR_LOGE(spiDevice->Transmit(&t, SPIFlags::POLLED), TAG, "PCF error");
	return Result::Ok;
}

uint8_t PCF2123::bcd_decode(uint8_t bcd)
{
	return (bcd >> 4) * 10 + (bcd & 0x0F);
}

uint8_t PCF2123::bcd_encode(uint8_t dec)
{
	return ((dec / 10) << 4) | (dec % 10);
}

Result PCF2123::DeviceSetConfig(IDeviceConfig &config)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(config.getProperty("spiDevice", &spiDeviceKey));
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result PCF2123::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(deviceManager->getDeviceByKey<SpiDevice>(spiDeviceKey, spiDevice));
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result PCF2123::DeviceInit()
{
	ContextLock lock(mutex);
	PCF2123_CtrlRegs regs;

	RETURN_ON_ERR(reset());
	RETURN_ON_ERR(ctrl_get(&regs));
	regs.ClearAll();
	RETURN_ON_ERR(ctrl_set(&regs, true, true, false));

	//TODO: Should we wait for clock integrity?

	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
}

Result PCF2123::TimeGet(DateTime &time)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR_LOGE(DeviceCheckStatus(DeviceStatus::Ready), TAG, "Device not ready");
	uint8_t buf[7] = {0};
	RETURN_ON_ERR(this->rxt(REG_TIME_DATE_ADDR, RXT_READ, buf, sizeof(buf))); // @TODO error check toevoegen

	int sec = bcd_decode(buf[0]		& 0x7F) ;   // 0-59
	int min = bcd_decode(buf[1]		& 0x7F) ;	// 0-59
	int hour = bcd_decode(buf[2]	& 0x2F) ;	// 0-24 (24h mode!)
	int days = bcd_decode(buf[3]	& 0x2F) ;   // 1-31
	int months = bcd_decode(buf[5]	& 0x1F) ;   // 1-12
	int years = bcd_decode(buf[6]	& 0xFF) ;   // 0-99

	years += 2000;	// Start at the year 2000
	char tbuf[32];
	sprintf(tbuf, "%04d-%02d-%02d %02d:%02d:%02d", years, months, days, hour, min, sec);
	time = DateTime::FromString(std::string(tbuf), "%Y-%m-%d %H:%M:%S", DateTimeMode::UTC);
	//ESP_LOGI(TAG, "TimeGet UTC = %s", tbuf);
	//TODO: Should we wait for clock integrity?
	return Result::Ok;
}

Result PCF2123::TimeSet(DateTime &new_time)
{
	ContextLock lock(mutex);
	RETURN_ON_ERR(DeviceCheckStatus(DeviceStatus::Ready));
	int sec 	= 0;
	int min 	= 0;
	int hour 	= 0;
	int days 	= 0;
	int months 	= 0;
	int years 	= 0;

	std::string utcString = new_time.toString("%Y-%m-%d %H:%M:%S", DateTimeMode::UTC).c_str();
	sscanf(utcString.c_str(), "%d-%d-%d %d:%d:%d", &years, &months, &days, &hour, &min, &sec);
	//ESP_LOGI(TAG, "TimeSet UTC = %s", utcString.c_str());

	years -= 2000;	// Start at the year 2000

	uint8_t buf[7];
	buf[0] = bcd_encode(sec);
	buf[1] = bcd_encode(min);
	buf[2] = bcd_encode(hour);
	buf[3] = bcd_encode(days);
	buf[4] = 0;
	buf[5] = bcd_encode(months);
	buf[6] = bcd_encode(years);

	RETURN_ON_ERR(this->rxt(REG_TIME_DATE_ADDR, RXT_WRITE, buf, sizeof(buf)));
	return Result::Ok;
}

Result PCF2123::reset(void)
{
	uint8_t buf = 0x58;
	RETURN_ON_ERR(rxt(REG_CTRL1_ADDR, RXT_WRITE, &buf, sizeof(buf)));
	return Result::Ok;
}

Result PCF2123::ctrl_get(PCF2123_CtrlRegs *regs)
{
	uint8_t buf[2];
	RETURN_ON_ERR(rxt(REG_CTRL1_ADDR, RXT_READ, buf, sizeof(buf)));
	regs->ctrl[0] = buf[0];
	regs->ctrl[1] = buf[1];
	return Result::Ok;
}

Result PCF2123::ctrl_set(PCF2123_CtrlRegs *regs, bool set_ctrl1, bool set_ctrl2, bool mask_alarms)
{
	uint8_t buf[2];
	if (mask_alarms)
		regs->mask_alarms();

	if (set_ctrl1 && set_ctrl2)
	{
		buf[0] = regs->ctrl[0];
		buf[1] = regs->ctrl[1];
		RETURN_ON_ERR(rxt(REG_CTRL1_ADDR, RXT_WRITE, buf, 2));
	}

	else if (set_ctrl1)
	{
		buf[0] = regs->ctrl[0];
		RETURN_ON_ERR(rxt(REG_CTRL1_ADDR, RXT_WRITE, buf, 1));
	}

	else if (set_ctrl2)
	{
		buf[0] = regs->ctrl[1];
		RETURN_ON_ERR(rxt(REG_CTRL2_ADDR, RXT_WRITE, buf, 1));
	}
	return Result::Ok;
}
