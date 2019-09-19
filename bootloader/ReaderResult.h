#pragma once
#include <stm32f4xx_hal.h>
#include "OperationType.h"
#include <stdlib.h>

namespace Firmware
{
	class ReaderResult
	{
	public:
		ReaderResult(OperationType type);
		uint8_t sizeBuffer;
		void* data;
		bool isOK;
		bool isEnd;
	};
}

