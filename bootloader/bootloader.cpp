#include <stm32f4xx_hal.h>
#include <stm32_hal_legacy.h>
#include <stm32f4xx_hal_flash.h>
#include "FirmwareReaderFromSD.h"
#include "FirmwareReaderFromFlash.h"

using namespace Firmware;


extern "C" void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}


#pragma region defines
#define FLASH_USER_SECTOR_START 2
#define FLASH_SECTORS_COUNT 12


#define FLASH_PAGE_SIZE		2048 						//2 Kbyte per page
#define FLASH_START_ADDR	0x08000000					//Origin
#define FLASH_MAX_SIZE		0x00080000					//Max FLASH size - 512 Kbyte
#define FLASH_END_ADDR		(FLASH_START_ADDR + FLASH_MAX_SIZE)		//FLASH end address
#define FLASH_BOOT_START_ADDR	(FLASH_START_ADDR)				//Bootloader start address
#define FLASH_BOOT_SIZE		0x00008000					//32 Kbyte for bootloader
#define FLASH_USER_START_ADDR	(FLASH_BOOT_START_ADDR + FLASH_BOOT_SIZE)	//User application start address
#define FLASH_USER_SIZE (FLASH_MAX_SIZE - FLASH_BOOT_SIZE)   //all free memory for user application
#define FLASH_OTHER_START_ADDR	(FLASH_MSD_START_ADDR + FLASH_MSD_SIZE)		//Other free memory start address
#define FLASH_OTHER_SIZE	(FLASH_END_ADDR - FLASH_OTHER_START_ADDR)	//Free memory size
#define APP_BLOCK_TRANSFER_SIZE 512
#define SIZE_OF_U32 sizeof(uint32_t)

#pragma endregion


SD_HandleTypeDef hsd;
void SystemClock_Config(void);
static void SDIO_SD_Init(void);
static void GPIO_Init(void);


void GoToUserApp(void);
void PeriphDeInit(void);
void Copy(IFirmwareReader* sdReader, IFirmwareReader* flashReader);
void check(IFirmwareReader* reader, IFirmwareReader* writer, void(*copy)(IFirmwareReader*, IFirmwareReader*));

void boot(void);
int main()
{
	HAL_Init();
	SystemClock_Config();
	
	GPIO_Init();
	SDIO_SD_Init();
	FATFS_Init();	

	//boot();
	TCHAR file[] = { 65, 80, 80, 46, 98, 105, 110, 0 };     //APP.bin
	IFirmwareReader* sdReader = new FirmwareReaderFromSD(file);	
	IFirmwareReader* flashReader = new FirmwareReaderFromFlash(FLASH_USER_START_ADDR);
	
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);	
	
	if (sdReader->GetSize() != flashReader->GetSize())
	{
		Copy(sdReader, flashReader);
	}
	else
	{			
		check(sdReader, flashReader, &Copy);
	}
	PeriphDeInit();
	GoToUserApp();
}

void Copy(IFirmwareReader* sdReader, IFirmwareReader* flashReader)
{
	sdReader->Reset();
	flashReader->Reset();
	
	ReaderResult* result;
	while (true)
	{
		result = sdReader->Read(DoubleWord);
		
		if (result->isOK)
		{
			flashReader->Write(DoubleWord, *(volatile uint64_t*)result->data);
			if (result->isEnd)
				break;
		}
		
		if (!result->isOK && result->isEnd)
		{
			while (true)
			{
				result = sdReader->Read(Byte);
		
				if (result->isOK)
				{
					flashReader->Write(Byte, *(volatile uint8_t*)result->data);
					if (result->isEnd)
						break;
				}
			}
		}
	}
}
	
void check(IFirmwareReader* sdReader, IFirmwareReader* flashReader, void(*copy)(IFirmwareReader*, IFirmwareReader*))
{
	ReaderResult* sdResult;
	ReaderResult* flashResult;
	
	while (true)
	{
		sdResult = sdReader->Read(DoubleWord);
		flashResult = flashReader->Read(DoubleWord);
		
		if ((!sdResult->isOK) && sdResult->isEnd)
		{
			while (true)
			{
				sdResult = sdReader->Read(Byte);
				flashResult = flashReader->Read(Byte);				
				
				if (*(volatile uint8_t*)(sdResult->data) != *(volatile uint8_t*)(flashResult->data))
				{
					//copy
					copy(sdReader, flashReader);
					break;
				}
				
				if (sdResult->isEnd)
					break;
			}
			break;
		}
		
		
		if (*(volatile uint64_t*)(sdResult->data) != *(volatile uint64_t*)(flashResult->data))
		{
			//copy
			copy(sdReader, flashReader);
			break;
		}
	}
}



void PeriphDeInit(void)
{
	HAL_SD_DeInit(&hsd);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
	HAL_DeInit();
}


void GoToUserApp(void)
{	
	uint32_t appJumpAddress;
	void(*GoToApp)(void);

	appJumpAddress = *((volatile uint32_t*)(FLASH_USER_START_ADDR + 4));
	GoToApp = (void(*)(void))(appJumpAddress);
	__disable_irq();
	SCB->VTOR = FLASH_USER_START_ADDR;
	__set_MSP(*((volatile uint32_t*) FLASH_USER_START_ADDR));         //stack pointer (to RAM) for USER app in this address
	
	PeriphDeInit();
	GoToApp();
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
	hsd.Init.ClockDiv = 1;
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
