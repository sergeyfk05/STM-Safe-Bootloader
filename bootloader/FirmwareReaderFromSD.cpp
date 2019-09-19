#include "FirmwareReaderFromSD.h"
#include <stm32f4xx_hal.h>
namespace Firmware
{
	FirmwareReaderFromSD::FirmwareReaderFromSD(const TCHAR* file_name)
		: mIsInit(false)
	{
		for (uint16_t i = 0; i < 258; i++)
		{
			mFileName[i] = file_name[i];
			
			if (file_name[i] == 0)
				break;
		}
	}
	
	ReaderResult* FirmwareReaderFromSD::Read(OperationType typeRead)
	{
		ReaderResult* result = new ReaderResult(typeRead);
		result->isOK = false;
		result->isEnd = false;
		
		if (!mIsInit)
			if(!Init())
				return result;
		
		UINT readBytes;		
		uint8_t g = typeRead;
		FRESULT readResult = f_read(&mAppFile, (void*)result->data, typeRead, &readBytes);
		
		if (readResult == FR_OK)
			result->isOK = true;
		
		if ((readBytes != typeRead))
			result->isEnd = true;
		
		return result;
	}
	
	bool FirmwareReaderFromSD::Write(OperationType typeWrite, uint64_t value)
	{
		//it's readonly class
		return false;
	}
	
	bool FirmwareReaderFromSD::ShiftPointer(int64_t shift)
	{
		if (shift == 0)
			return true;
		
		if (shift < 0)
		{
			if (mAppFile.fptr + shift < 0)
				return false;
			
			f_lseek(&mAppFile, mAppFile.fptr + shift);
			return true;
		}
		else
		{
			if (mAppFile.fptr + shift > mAppFile.obj.objsize)
				return false;
			
			f_lseek(&mAppFile, mAppFile.fptr + shift);
			return true;
		}
	}
	
	void FirmwareReaderFromSD::Reset()
	{
		if (!mIsInit && !Init())
			return;
		
		f_lseek(&mAppFile, 0);
	}
	
	bool FirmwareReaderFromSD::Init()
	{
		TCHAR drive  = (TCHAR)0;
		FRESULT status = f_mount(&mFatfsObj, &drive, 1);
		if (status != FR_OK)
			return false;
		
		status = f_open(&mAppFile, mFileName, FA_READ);
		if (status != FR_OK)
			return false;
		
		
		mIsInit = true;
		return true;
	}
	
}