#include "FirmwareReaderFromFlash.h"
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_flash.h>

namespace Firmware
{
	FirmwareReaderFromFlash::FirmwareReaderFromFlash(uint32_t startAddress)
		: mStartAddress(startAddress), mOffset(0)
	{
		
	}
	
	ReaderResult* FirmwareReaderFromFlash::Read(OperationType typeRead)
	{
		ReaderResult* result = new ReaderResult(typeRead);
		
		if (mOffset + typeRead > GetSize())
		{
			result->isEnd = true;
			result->isOK = false;
			return result;
		}
		
		switch (typeRead)
		{
		case Byte: *(uint8_t*)result->data = *((volatile uint8_t*)(mStartAddress + mOffset));
			break;
		case HalfWord: *(uint16_t*)result->data = *((volatile uint16_t*)(mStartAddress + mOffset));
			break;
		case Word: *(uint32_t*)result->data = *((volatile uint32_t*)(mStartAddress + mOffset));
			break;
		case DoubleWord: *(uint64_t*)result->data = *((volatile uint64_t*)(mStartAddress + mOffset));
			break;
		}
		
		mOffset += typeRead;
		
		result->isOK = true;		
		result->isEnd = GetSize() >= mOffset ? true : false;
		
		return result;
	}
		
	bool FirmwareReaderFromFlash::Write(OperationType typeWrite, uint64_t value)
	{
		if (mOffset + typeWrite > GetSize())
			return false;
		
		uint16_t type;
		switch (typeWrite)
		{
		case Byte: type = 0;
			break;
		case HalfWord: type = 1;
			break;
		case Word: type = 2;
			break;
		case DoubleWord: type = 3;
			break;
		}
		
		for (uint8_t i = 0; i <= type; i++)
		{
			//ToDo: clear pages
		}
		
		HAL_FLASH_Program(type, mStartAddress + mOffset, value);
		
		mOffset += typeWrite;
		return true;
	}
	
	void FirmwareReaderFromFlash::Reset()
	{
		mOffset = 0;
	}
	
	bool FirmwareReaderFromFlash::ShiftPointer(int64_t shift)
	{
		if (shift == 0)
			return true;
		
		if (shift < 0)
		{
			if (mOffset + shift < 0)
				return false;
			
			mOffset += shift;
			return true;
		}
		else
		{
			if (mOffset + shift > GetSize())
				return false;
			
			mOffset += shift;
			return true;
		}
	}
}