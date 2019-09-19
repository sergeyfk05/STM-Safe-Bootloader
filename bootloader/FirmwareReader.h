#pragma once

#include <stm32f4xx_hal.h>
#include "TypeRead.h"
#include "ReaderResult.h"
#include "ffconf.h"
#include "fatfs.h"

#ifdef __cplusplus
extern "C" {
#endif
	namespace Firmware
	{
		class FirmwareReader
		{
		public:
			/**
			 * @brief  Constructor
			 * @param  fileName  Name of firmware file.
			 */
			FirmwareReader(const TCHAR* fileName);

			/**
			  * @brief  Read the firmware
			  * @param  typeRead  Indicate the count of read byte need.
			  * 
			  * @retval ReaderResult firmware bytes
			  */
			ReaderResult* Read(TypeRead typeRead);
			
			void Reset();
		
		private:
			FATFS mFatfsObj;
			FIL mAppFile;
			TCHAR mFileName[258];
			bool mIsInit;
			char mBuffer[512];
			
			/**
			  * @brief  Init variables for read firmware.
			  * 
			  * @retval bool If true - all is OK. If false - error.
			  */
			bool Init();
		};
	}
#ifdef __cplusplus
}
#endif
