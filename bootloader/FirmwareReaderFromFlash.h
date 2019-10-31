#pragma once

#include <stm32f4xx_hal.h>
#include "IFirmwareReader.h"

#define SECTORS_COUNT 12
#define SECTOR_ADDR0 0x08000000
#define SECTOR_ADDR1 0x08004000
#define SECTOR_ADDR2 0x08008000
#define SECTOR_ADDR3 0x0800C000
#define SECTOR_ADDR4 0x08010000
#define SECTOR_ADDR5 0x08020000
#define SECTOR_ADDR6 0x08040000
#define SECTOR_ADDR7 0x08060000
#define SECTOR_ADDR8 0x08080000
#define SECTOR_ADDR9 0x080A0000
#define SECTOR_ADDR10 0x080C0000
#define SECTOR_ADDR11 0x080E0000
#define MAX_FLASH_ADDR 0x080FFFFF
#define START_SECTOR FLASH_SECTOR_2

#ifdef __cplusplus
extern "C" {
#endif
	
	namespace Firmware
	{
		class FirmwareReaderFromFlash : public IFirmwareReader
		{
		public:
			/**
			 * @brief  Constructor
			 */
			FirmwareReaderFromFlash(uint32_t startAddress);
			
			/**
			  * @brief  Read the firmware.
			  * @param  typeRead  Indicate the count of read byte need.
			  * 
			  * @retval ReaderResult firmware bytes
			  */
			ReaderResult* Read(OperationType typeRead);
			
			/**
			  * @brief  Write the firmware.
			  * @param  typeRead  Indicate the count of write byte need.
			  * 
			  * @retval ReaderResult firmware bytes
			  */
			bool Write(OperationType typeWrite, uint64_t value);
			
			
			/**
			  * @brief  Shift pointer of firmware.
			  * @param  shift  The number of bytes the pointer will shift
			  * 
			  * @retval bool If true - all is OK. If false - error.
			  */
			bool ShiftPointer(int64_t shift);
			
			/**
			  * @brief  Reset state.
			  */
			void Reset();
			
			
			~FirmwareReaderFromFlash();
		
		private:
			const uint32_t mStartAddress;
			uint32_t mOffset;	
			bool mIsInit;
			bool Init();
		};
	}
	
#ifdef __cplusplus
}
#endif
