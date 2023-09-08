#pragma once
#include <functional>


enum class PinModes
{
	PIN_INPUT,
	PIN_OUTPUT,
};


enum class ISRModes
{
	PIN_DISABLE = 0,	/*!< Disable GPIO interrupt                             */
	PIN_POSEDGE = 1,	/*!< GPIO interrupt type : rising edge                  */
	PIN_NEGEDGE = 2,	/*!< GPIO interrupt type : falling edge                 */
	PIN_ANYEDGE = 3,	/*!< GPIO interrupt type : both rising and falling edge */
	PIN_LOW_LEVEL = 4,	/*!< GPIO interrupt type : input low level trigger      */
	PIN_HIGH_LEVEL = 5,	/*!< GPIO interrupt type : input high level trigger     */
};

// This is more difficoult than I thought, so not for now. Maybee later

//template<typename TPINS>
//class IGPIO
//{
//	Event<IGPIO, TPINS> OnPinChange;
//public:
//	
//	virtual void SetPinsMode(TPINS mask, PinModes mode)	= 0;
//	virtual void SetPins(TPINS mask, TPINS value) = 0;
//	virtual void GetPins(TPINS mask, TPINS* value) = 0;
//	virtual void SetInterrupts(TPINS mask, ISRModes mode) = 0;
//	
//	void AddHandler(const std::function<void(IGPIO*, const TPINS&)>& handler) {
//		OnPinChange.AddHandler(handler);
//	}
//	
//	void RemoveHandler(const std::function<void(IGPIO*, const TPINS&)>& handler) {
//		OnPinChange.RemoveHandler(handler);
//	}
//};

