#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h" /* defines SD_Driver as external */

	/* USER CODE BEGIN Includes */

	/* USER CODE END Includes */

	extern uint8_t retSD; /* Return value for SD */
	extern char SDPath[4]; /* SD logical drive path */
	extern FATFS SDFatFS; /* File system object for SD logical drive */
	extern FIL SDFile; /* File object for SD */

	void MX_FATFS_Init(void);
	 
#ifdef __cplusplus
}
#endif
