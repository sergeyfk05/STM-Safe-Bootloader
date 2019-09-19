#pragma once

#include <stm32f4xx_hal.h>
#include <IReader.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	namespace Firmware
	{
		class FirmwareReaderFromFlash : public IReader
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
			
			void Reset();
		
		private:
			const uint32_t mStartAddress;
			uint32_t mOffset;
			
			size_t GetSize();
		};
	}
	
#ifdef __cplusplus
}
#endif
