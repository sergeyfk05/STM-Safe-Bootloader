#pragma once
#include <stm32f4xx_hal.h>
#include "OperationType.h"
#include <stdlib.h>

namespace Firmware
{
	
	enum FR_Status
	{
		OK,
		Error,
		End,
		TooMuchBytes
	};
	
	class ReaderResult
	{
	public:
		ReaderResult(OperationType type);
		//uint8_t sizeBuffer;
		void* data;
		FR_Status status;
		~ReaderResult();
	};
	

}

