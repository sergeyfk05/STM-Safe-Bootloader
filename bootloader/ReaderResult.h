#pragma once
#include <stm32f4xx_hal.h>

namespace Firmware
{
	class ReaderResult
	{
	public:
		char data[8];
		bool isOK;
		bool isEnd;
	};
}

