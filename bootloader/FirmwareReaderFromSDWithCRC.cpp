#include "FirmwareReaderFromSDWithCRC.h"

namespace Firmware
{
	FirmwareReaderFromSDWithCRC::FirmwareReaderFromSDWithCRC(const TCHAR* appFileName)
		: FirmwareReaderFromSD(appFileName)
	{
	}
	bool FirmwareReaderFromSDWithCRC::CheckCorrectness()
	{
		if (!Init())
			return false;
		
		//clock for CRC
		RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
		//reset CRC
		CRC->CR = 0;	
		
		uint32_t buffer = 0xFFFFFFFF;
		uint8_t lenOfRead = 4;
		UINT readedBytes = lenOfRead;		
		
		
		FSIZE_t startPointer = mAppFile.fptr;
		Reset();
		while ((f_read(&mAppFile, &buffer, lenOfRead, &readedBytes) == FR_OK) && (readedBytes != 0))
		{
			
			CRC->DR = buffer;
			
			if (readedBytes < lenOfRead)
				break;
		}
		//shift pointer to start position
		Reset();
		ShiftPointer(startPointer);
		
		uint32_t appCRC = CRC->DR;		
		uint32_t trueCRC;
		
		//reset CRC for security
		CRC->CR = 0;
		
		FIL crcFile;
		TCHAR crcFileName[] = { 65, 80, 80, 46, 99, 104, 101, 99, 107, 115, 117, 109, 0 };                //APP.checksum
		//TCHAR crcFileName[] = { 65, 80, 80, 46, 99, 114, 99, 10, 0 };                 //APP.crc
		FRESULT status = f_open(&crcFile, crcFileName, FA_READ);
		if (status != FR_OK)
			return false;//failed open checksum File
		
		if (f_size(&crcFile) != 4)
			return false;//invalid checksum file size;
		
		f_read(&crcFile, &trueCRC, 4, &readedBytes);
		
		if (readedBytes != 4)
			return false;//failed read checksum file
	
		
		return appCRC == trueCRC ? true : false;
	}
}