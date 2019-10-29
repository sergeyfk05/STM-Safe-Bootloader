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
		
		//check Init
		if (!mIsInit)
			if (!Init())
			{
				result->status = Error;
				return result;
			}
		
		UINT readBytes;		
		uint8_t g = typeRead;
		FRESULT readResult = f_read(&mAppFile, (void*)result->data, typeRead, &readBytes);
		
		//set status
		if(readResult == FR_OK && readBytes == typeRead)
		{
			result->status = OK;
		}
		else if(readResult != FR_OK)
		{
			result->status = Error;
		}
		else if(readResult == FR_OK && readBytes == 0)
			result->status = End; 
		else if(readResult == FR_OK && readBytes != typeRead)
		{
			result->status = TooMuchBytes;
			this->ShiftPointer(-1 * readBytes);
		}
		
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
			//check for going outside the address
			if (mAppFile.fptr + shift < 0)
				return false;
			
			f_lseek(&mAppFile, mAppFile.fptr + shift);
			return true;
		}
		else
		{
			//check for going outside the address
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
		//mount drive
		TCHAR drive  = (TCHAR)0;
		FRESULT status = f_mount(&mFatfsObj, &drive, 1);
		if (status != FR_OK)
			return false;
		
		//open firmware file
		status = f_open(&mAppFile, mFileName, FA_READ);
		if (status != FR_OK)
			return false;
		
		
		mIsInit = true;
		return true;
	}
	
	FirmwareReaderFromSD::~FirmwareReaderFromSD()
	{
		//close file
		f_close(&mAppFile);
		TCHAR drive  = (TCHAR)0;
		
		//unmount drive
		f_mount(NULL, &drive, 0);
		
		//free memory
		free(&mFatfsObj);
	}
	
}