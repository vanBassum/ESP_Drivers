#pragma once

#include <string>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_base.h"


class Task
{
	void* arg = NULL;
	std::string name = "New task";
	portBASE_TYPE priority = 0;
	portSHORT stackDepth = configMINIMAL_STACK_SIZE;
	TaskHandle_t taskHandle = NULL;
	Action<Task*, void*> callback;
		
	static void TaskFunction(void* parm)
	{
		Task* t = static_cast<Task*>(parm);
		t->callback.Invoke(t, t->arg);
		t->taskHandle = NULL;	
		vTaskDelete(NULL);
	}
		
public:
		
	Task()
	{
			
	}
		
	~Task()
	{
		if (taskHandle != NULL)
			vTaskDelete(taskHandle);
	}
		
	void Init(std::string name, portBASE_TYPE priority, portSHORT stackDepth)
	{
		this->name = name;
		this->priority = priority;
		this->stackDepth = stackDepth;
		if (this->stackDepth < configMINIMAL_STACK_SIZE)
			this->stackDepth = configMINIMAL_STACK_SIZE;
	}
		
	template<typename T>
		void Bind(T* instance, void(T::* mp)(Task*, void*))
	{
		callback.Bind(instance, mp);
	}

	void Bind(void(*fp)(Task*, void*))
	{
		callback.Bind(fp);
	}
		
	void Run(void* arg = NULL)
	{
		if (taskHandle != NULL)
			vTaskDelete(taskHandle);
		this->arg = arg;
		xTaskCreate(&TaskFunction, name.c_str(), stackDepth, this, priority, &taskHandle);
	}
		
	void RunPinned(const BaseType_t core)
	{
		if (taskHandle != NULL)
			vTaskDelete(taskHandle);
		xTaskCreatePinnedToCore(&TaskFunction, name.c_str(), stackDepth, this, priority, &taskHandle, core);
	}
		
#ifdef configUSE_TASK_NOTIFICATIONS
		
	bool NotifyWait(uint32_t* pulNotificationValue, int timeout = portMAX_DELAY)
	{
		return xTaskNotifyWait(0x0000, 0xFFFF, pulNotificationValue, timeout) == pdPASS;
	}

	void Notify(uint32_t bits)
	{
		xTaskNotify(taskHandle, bits, eSetBits);
	}

	void NotifyFromISR(uint32_t bits)
	{
		xTaskNotifyFromISR(taskHandle, bits, eSetBits, NULL);
	}

	void NotifyFromISR(uint32_t bits, BaseType_t* pxHigherPriorityTaskWoken)
	{
		xTaskNotifyFromISR(taskHandle, bits, eSetBits, pxHigherPriorityTaskWoken);
	}
		
#endif // configUSE_TASK_NOTIFICATIONS

	static int GetCurrentCoreID()
	{
		return xPortGetCoreID();
	}
		
};
