#pragma once
#include <stm32f4xx_hal.h>
#include "OperationType.h"
#include "ReaderResult.h"

namespace Firmware
{
	class IFirmwareReader
	{
	public:
		ReaderResult* Read(OperationType typeRead);
		bool Write(OperationType typeWrite, uint64_t value);
		bool ShiftPointer(int64_t shift);			
		void Reset();
		
	};
}
