#include <stm32f4xx_hal.h>
#include <stm32_hal_legacy.h>
#include <stm32f4xx_hal_flash.h>

#include "ffconf.h"
#include "fatfs.h"



extern "C" void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}


#pragma region defines

#define FLASH_PAGE_SIZE		2048 						//2 Kbyte per page
#define FLASH_START_ADDR	0x08000000					//Origin
#define FLASH_MAX_SIZE		0x00080000					//Max FLASH size - 512 Kbyte
#define FLASH_END_ADDR		(FLASH_START_ADDR + FLASH_MAX_SIZE)		//FLASH end address
#define FLASH_BOOT_START_ADDR	(FLASH_START_ADDR)				//Bootloader start address
#define FLASH_BOOT_SIZE		0x00010000					//64 Kbyte for bootloader
#define FLASH_USER_START_ADDR	(FLASH_BOOT_START_ADDR + FLASH_BOOT_SIZE)	//User application start address
//#define FLASH_USER_SIZE		0x00032000					//200 Kbyte for user application
#define FLASH_USER_SIZE (FLASH_MAX_SIZE - FLASH_BOOT_SIZE)   //all free memory for user application
//#define FLASH_MSD_START_ADDR	(FLASH_USER_START_ADDR + FLASH_USER_SIZE)	//USB MSD start address
//#define FLASH_MSD_SIZE		0x00032000					//200 Kbyte for USB MASS Storage
#define FLASH_OTHER_START_ADDR	(FLASH_MSD_START_ADDR + FLASH_MSD_SIZE)		//Other free memory start address
#define FLASH_OTHER_SIZE	(FLASH_END_ADDR - FLASH_OTHER_START_ADDR)	//Free memory size
#define APP_BLOCK_TRANSFER_SIZE 512
#define SIZE_OF_U32 sizeof(uint32_t)

#pragma endregion


SD_HandleTypeDef hsd;


void SystemClock_Config(void);
static void SDIO_SD_Init(void);
static void GPIO_Init(void);
void SystemClock_Config(void);
void GoToUserApp(void);
void CopyAppToUserMemory(FIL* appFile, uint64_t appSize);
void PeriphDeInit(void);


int main()
{
	HAL_Init();
	SystemClock_Config();
	
	GPIO_Init();
	SDIO_SD_Init();
	FATFS_Init();
	
	
	FRESULT result;
	HAL_Delay(2U);

	// смонтировать диск
	FATFS FATFS_Obj;
	uint16_t n = 48;
	
	
	TCHAR drive  = (TCHAR)0;
	result = f_mount(&FATFS_Obj, &drive, 1);
	if (result != FR_OK)
	{
		//printf("Ошибка монтирования диска %d\r\n", result);
	}
	n = 50;
}


void PeriphDeInit(void)
{
	HAL_SD_DeInit(&hsd);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
	HAL_DeInit();
}

void boot(void)
{
	
	FATFS FATFS_Obj;
	TCHAR drive  = (TCHAR)0;
	FRESULT FATFS_Status = f_mount(&FATFS_Obj, &drive, 1);
	if (FATFS_Status == FR_OK)
	{
		FIL appFile;
		TCHAR file = (TCHAR)"/APP.BIN";
		FRESULT FILE_Status = f_open(&appFile, &file, FA_READ);
		if (FILE_Status == FR_OK)
		{
			uint64_t appSize = f_size(&appFile);

			uint64_t i;
			for (i = 0; i < appSize; i++) //Byte-to-byte compare files in MSD_MEM and USER_MEM
				{
					UINT readBytes;
					char appBuffer;
					f_read(&appFile, &appBuffer, 1, &readBytes);

					if (*((volatile uint8_t*)(FLASH_USER_START_ADDR + i)) != appBuffer[0]) 
					{
						//if byte of USER_MEM != byte of MSD_MEM
						break;
					}
				}

			if (i != appSize)//=> was done "break" instruction in for(;;) cycle => new firmware in MSD_FLASH
			{
				CopyAppToUserMemory(&appFile, appSize);
			}

			FILE_Status = f_close(&appFile);
			TCHAR drive  = (TCHAR)0;
			FATFS_Status = f_mount(NULL, &drive, 1);

			
			PeriphDeInit();
			GoToUserApp();
		}
		else //if FILE_Status != FR_OK
			{
				if (FILE_Status == FR_NO_FILE)
				{
					//No file error
				}
				else //if FILE_Status != FR_NO_FILE
					{
						//Other error
					}
				TCHAR drive  = (TCHAR)0;
				FATFS_Status = f_mount(NULL, &drive, 1);
				while (true);
			}
	}
	else //FATFS_Status != FR_OK
		{
			//FatFS mount error
			while(true);
		}
}

void CopyAppToUserMemory(FIL* appFile, uint64_t appSize)
{
	f_lseek(appFile, 0);  //Go to the fist position of file

	UINT readBytes;
	uint64_t appTailSize = appSize % APP_BLOCK_TRANSFER_SIZE;
	uint64_t appBodySize = appSize - appTailSize;
	uint64_t appAddrPointer = 0;

	for (uint64_t i = 0; i < ((appSize / FLASH_PAGE_SIZE) + 1); i++) //Erase n + 1 pages for new application
		{
			
			while (FLASH_GetStatus() != FLASH_COMPLETE) ;
			FLASH_ErasePage(FLASH_USER_START_ADDR + i * FLASH_PAGE_SIZE);
		}

	
	char appBuffer[APP_BLOCK_TRANSFER_SIZE];
	for (uint64_t i = 0; i < appBodySize; i += APP_BLOCK_TRANSFER_SIZE)
	{
		/*
		 * For example, size of File1 = 1030 bytes
		 * File1 = 2 * 512 bytes + 6 bytes
		 * "body" = 2 * 512, "tail" = 6
		 * Let's write "body" and "tail" to MCU FLASH byte after byte with 512-byte blocks
		 */
		f_read(appFile, appBuffer, APP_BLOCK_TRANSFER_SIZE, &readBytes);  //Read 512 byte from file
		for(uint64_t j = 0 ; j < APP_BLOCK_TRANSFER_SIZE ; j += SIZE_OF_U32) //write 512 byte to FLASH
		{
			while (FLASH_GetStatus() != FLASH_COMPLETE) ;
			FLASH_ProgramWord(FLASH_USER_START_ADDR + i + j, *((volatile uint32_t*)(appBuffer + j)));
		}
		appAddrPointer += APP_BLOCK_TRANSFER_SIZE;  //pointer to current position in FLASH for write
	}

	f_read(appFile, appBuffer, appTailSize, &readBytes);  //Read "tail" that < 512 bytes from file

	while((appTailSize % SIZE_OF_U32) != 0)		//if appTailSize MOD 4 != 0 (seems not possible, but still...)
	{
		appTailSize++; 				//increase the tail to a multiple of 4
		appBuffer[appTailSize - 1] = 0xFF; 	//and put 0xFF in this tail place
	}

	for (uint64_t i = 0; i < appTailSize; i += SIZE_OF_U32) //write "tail" to FLASH
		{
			while (FLASH_GetStatus() != FLASH_COMPLETE) ;
			FLASH_ProgramWord(FLASH_USER_START_ADDR + appAddrPointer + i, *((volatile uint32_t*)(appBuffer + i))); 
		}
}


void GoToUserApp(void)
{	
	uint32_t appJumpAddress;
	void(*GoToApp)(void);

	appJumpAddress = *((volatile uint32_t*)(FLASH_USER_START_ADDR + 4));
	GoToApp = (void(*)(void))appJumpAddress;
	SCB->VTOR = FLASH_USER_START_ADDR;
	__set_MSP(*((volatile uint32_t*) FLASH_USER_START_ADDR));   //stack pointer (to RAM) for USER app in this address
	GoToApp();    		                        //Jump to main firmware	
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage 
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
	                            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		//Error_Handler();
	}
}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void SDIO_SD_Init(void)
{

	__HAL_RCC_SDIO_CLK_ENABLE();  
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 
	                      | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	
	
	hsd.Instance = SDIO;
	hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
	hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
	hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
	hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
	hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
	hsd.Init.ClockDiv = 0;

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
}
