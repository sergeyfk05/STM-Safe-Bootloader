#include "FirmwareReaderFromFlash.h"
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_flash.h>
#include <math.h>

namespace Firmware
{
	FirmwareReaderFromFlash::FirmwareReaderFromFlash(uint32_t startAddress)
		: mStartAddress(startAddress)
		, mOffset(0)
		, mIsInit(false)
	{
		return;
	}
	
	bool FirmwareReaderFromFlash::Init()
	{	
		if (mIsInit)
			return true;
		
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);	
		
		FLASH_EraseInitTypeDef fei;
		uint32_t error;
		fei.TypeErase = FLASH_TYPEERASE_SECTORS;
		fei.Sector = START_SECTOR;
		fei.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		fei.NbSectors = 12 - START_SECTOR;
		HAL_FLASHEx_Erase(&fei, &error);
		
		
		mIsInit = true;
		HAL_FLASH_Lock();
		return true;
	}
	
	ReaderResult* FirmwareReaderFromFlash::Read(OperationType typeRead)
	{		
		ReaderResult* result = new ReaderResult(typeRead);
		
		//check for going outside the address
		if(mStartAddress + mOffset > MAX_FLASH_ADDR)
		{
			result->status = End;
			return result;
		}
		
		//convert OperationType to count of bytes
		switch(typeRead)
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
		
		//check for real readed bytes;
		result->status = this->ShiftPointer(typeRead) ? OK : TooMuchBytes;
		
		return result;
	}
		
	bool FirmwareReaderFromFlash::Write(OperationType typeWrite, uint64_t value)
	{		
		//init check
		if(!Init())
			return false;
		
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);	
		
		//check for going outside the address
		if(mStartAddress + mOffset + typeWrite > MAX_FLASH_ADDR)
			return false;
		
		
		//convert OperationType to FlashProgramType
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
		
		HAL_FLASH_Lock();
		return true;
	}
	
	void FirmwareReaderFromFlash::Reset()
	{
		//reset pointer
		mOffset = 0;
	}
	
	bool FirmwareReaderFromFlash::ShiftPointer(int64_t shift)
	{
		//check by zero
		if(shift == 0)
			return true;
		
		if (shift < 0)
		{
			//check for going outside the address
			if(mOffset + shift < 0)
				return false;
			
			mOffset += shift;
			return true;
		}
		else
		{
			//check for going outside the address
			if(mStartAddress + mOffset + shift > MAX_FLASH_ADDR + 1)
				return false;
			
			mOffset += shift;
			return true;
		}
	}
	
	FirmwareReaderFromFlash::~FirmwareReaderFromFlash()
	{
	}

}