#pragma once

#include "FirmwareReaderFromSD.h"

#ifdef __cplusplus
extern "C" {
#endif

	namespace Firmware
	{
		class FirmwareReaderFromSDWithCRC : public FirmwareReaderFromSD
		{
		public:
			
			FirmwareReaderFromSDWithCRC(const TCHAR* appFileName);
			
			/**
			  * @brief  Check correctness of the firmware.
			  * 
			  * @retval bool If true - firmware is correctness. If false - error.
			  */
			bool CheckCorrectness();
		};
	}
	
#ifdef __cplusplus
}
#endif
