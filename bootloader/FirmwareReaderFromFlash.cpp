#include "FirmwareReaderFromFlash.h"
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_flash.h>

namespace Firmware
{
	FirmwareReaderFromFlash::FirmwareReaderFromFlash(uint32_t startAddress)
		: mStartAddress(startAddress)
		, mOffset(0)
		, mIsInit(false)
	{

	}
	
	void FirmwareReaderFromFlash::Init()
	{
		int8_t startSector = -1;
		if ((mStartAddress >= SECTOR_ADDR0) &&(mStartAddress < SECTOR_ADDR1))
			startSector = 0;
		else if ((mStartAddress >= SECTOR_ADDR1) &&(mStartAddress < SECTOR_ADDR2))
			startSector = 1;
		else if ((mStartAddress >= SECTOR_ADDR2) &&(mStartAddress < SECTOR_ADDR3))
			startSector = 2;		
		else if ((mStartAddress >= SECTOR_ADDR3) &&(mStartAddress < SECTOR_ADDR4))
			startSector = 3;		
		else if ((mStartAddress >= SECTOR_ADDR4) &&(mStartAddress < SECTOR_ADDR5))
			startSector = 4;		
		else if ((mStartAddress >= SECTOR_ADDR5) &&(mStartAddress < SECTOR_ADDR6))
			startSector = 5;		
		else if ((mStartAddress >= SECTOR_ADDR6) &&(mStartAddress < SECTOR_ADDR7))
			startSector = 6;		
		else if ((mStartAddress >= SECTOR_ADDR7) &&(mStartAddress < SECTOR_ADDR8))
			startSector = 7;		
		else if ((mStartAddress >= SECTOR_ADDR8) &&(mStartAddress < SECTOR_ADDR9))
			startSector = 8;		
		else if ((mStartAddress >= SECTOR_ADDR9) &&(mStartAddress < SECTOR_ADDR10))
			startSector = 9;		
		else if ((mStartAddress >= SECTOR_ADDR10) &&(mStartAddress < SECTOR_ADDR11))
			startSector = 10;		
		else if (mStartAddress >= SECTOR_ADDR11)
			startSector = 11;
		
		if (startSector > -1)
			for (; startSector < SECTORS_COUNT; startSector++)
			{
				FLASH_Erase_Sector(startSector, FLASH_VOLTAGE_RANGE_3);
				if (FLASH_WaitForLastOperation(1000) != HAL_OK)
					break;
			}
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
		
		if (!mIsInit)
		{
			Init();
			mIsInit = true;
		}
		
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