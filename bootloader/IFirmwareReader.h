#pragma once
#include <stm32f4xx_hal.h>
#include "OperationType.h"
#include "ReaderResult.h"

namespace Firmware
{
	class IFirmwareReader
	{
	public:
		/**
		  * @brief  Read the firmware.
		  * @param  typeRead  Indicate the count of read byte need.
		  * 
		  * @retval ReaderResult firmware bytes
		  */
		virtual ReaderResult* Read(OperationType typeRead) = 0;
		/**
		  * @brief  Write the firmware.
		  * @param  typeRead  Indicate the count of write byte need.
		  * 
		  * @retval ReaderResult firmware bytes
		  */
		virtual bool Write(OperationType typeWrite, uint64_t value) = 0;
		/**
		  * @brief  Shift pointer of firmware.
		  * @param  shift  The number of bytes the pointer will shift
		  * 
		  * @retval bool If true - all is OK. If false - error.
		  */
		virtual bool ShiftPointer(int64_t shift) = 0;	
		/**
		  * @brief  Reset state.
		  */
		virtual void Reset() = 0;
		
		virtual ~IFirmwareReader()
		{
		}
		
	};
}
