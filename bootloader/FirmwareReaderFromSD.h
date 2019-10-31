#pragma once

#include <stm32f4xx_hal.h>
#include "IFirmwareReader.h"
#include "ffconf.h"
#include "fatfs.h"

#ifdef __cplusplus
extern "C" {
#endif
	namespace Firmware
	{
		class FirmwareReaderFromSD : public IFirmwareReader
		{
		public:
			/**
			 * @brief  Constructor
			 * @param  fileName  Name of firmware file.
			 */
			FirmwareReaderFromSD(const TCHAR* fileName);

			/**
			  * @brief  Read the firmware
			  * @param  typeRead  Indicate the count of read byte need.
			  * 
			  * @retval ReaderResult firmware bytes
			  */
			ReaderResult* Read(OperationType typeRead);
			
//			/**
//			  * @brief  Write the firmware.
//			  * @param  typeRead  Indicate the count of write byte need.
//			  * 
//			  * @retval ReaderResult firmware bytes
//			  */
//			bool Write(OperationType typeWrite, uint64_t value);			
			
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
			
			~FirmwareReaderFromSD();
		
		protected:
			FATFS mFatfsObj;
			FIL mAppFile;
			/**
			  * @brief  Init variables for read firmware.
			  * 
			  * @retval bool If true - all is OK. If false - error.
			  */
			bool Init();
		private:
			TCHAR mFileName[258];
			bool mIsInit;
		};
	}
#ifdef __cplusplus
}
#endif
