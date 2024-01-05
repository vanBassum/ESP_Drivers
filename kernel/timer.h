#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_base.h"
#include <functional>


class Timer
{
	std::function<void()> callback;
	TimerHandle_t xTimer = NULL;
		
	static void tCallback(TimerHandle_t xTimer)
	{
		Timer* t = static_cast<Timer*>(pvTimerGetTimerID(xTimer));
		if (t && t->callback) {
			t->callback();
		}
	}
		
public:
	Timer()
	{	

	}

	~Timer()
	{
		if (xTimer != NULL)
			xTimerDelete(xTimer, 0);
	}
		

	void SetHandler(const std::function<void()> callback)
	{
		this->callback = callback;
	}
		
	bool Init(const std::string name, TimeSpan period, bool autoReload = true)
	{
		TickType_t rtosTicks = period.GetMiliSeconds() / portTICK_PERIOD_MS;
		if (rtosTicks < 1)
			rtosTicks = 1;
		xTimer = xTimerCreate(name.c_str(), rtosTicks, autoReload, this, &tCallback);
		return xTimer != NULL;
	}
		
	bool Start(int timeout = portMAX_DELAY)
	{
		if (xTimer == NULL) return false;
		return xTimerStart(xTimer, timeout) == pdPASS;
	}

	bool Stop(int timeout = portMAX_DELAY)
	{
		if (xTimer == NULL) return false;
		return xTimerStop(xTimer, timeout) == pdPASS;
	}

	bool Reset(int timeout = portMAX_DELAY)
	{
		if (xTimer == NULL) return false;
		return xTimerReset(xTimer, timeout) == pdPASS;
	}

	bool IsRunning()
	{
		if (xTimer == NULL) return false;
		return xTimerIsTimerActive(xTimer) != pdFALSE;
	}
		
	bool SetPeriod(TimeSpan period, int timeout = portMAX_DELAY)
	{		
		if (xTimer == NULL) return false;
		TickType_t rtosTicks = period.GetTotalMiliSeconds() / portTICK_PERIOD_MS;
		if (rtosTicks < 1)
			rtosTicks = 1;
		return xTimerChangePeriod(xTimer, rtosTicks, timeout ) == pdPASS;
	}

	TimeSpan GetPeriod()
	{
		TimeSpan ts;
		if (xTimer != NULL)
			ts = TimeSpan::FromMiliseconds(xTimerGetPeriod(xTimer) * portTICK_PERIOD_MS);
		return ts;
	}
};