#include <stm32f4xx_hal.h>
#include <stm32_hal_legacy.h>
#include <stm32f4xx_hal_flash.h>
#include "FirmwareReaderFromSD.h"
#include "FirmwareReaderFromFlash.h"

#include "fatfs.h"

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
#define FLASH_BOOT_SIZE		0x00008000					//64 Kbyte for bootloader
#define FLASH_USER_START_ADDR	(FLASH_BOOT_START_ADDR + FLASH_BOOT_SIZE)	//User application start address
#define FLASH_USER_SIZE (FLASH_MAX_SIZE - FLASH_BOOT_SIZE)   //all free memory for user application
#define FLASH_OTHER_START_ADDR	(FLASH_MSD_START_ADDR + FLASH_MSD_SIZE)		//Other free memory start address
#define FLASH_OTHER_SIZE	(FLASH_END_ADDR - FLASH_OTHER_START_ADDR)	//Free memory size
#define APP_BLOCK_TRANSFER_SIZE 512
#define SIZE_OF_U32 sizeof(uint32_t)

#pragma endregion


SD_HandleTypeDef hsd;
void SystemClock_Config(void);


void GoToUserApp(void);
void PeriphDeInit(void);
void CopyFirmware(IFirmwareReader* reader, IFirmwareReader* writer);
void CheckFirmwareAndCopy(IFirmwareReader* source, IFirmwareReader* destination, void(*copyMethod)(IFirmwareReader*, IFirmwareReader*));

void boot(void);
int main()
{
	HAL_Init();
	SystemClock_Config();
	
	FATFS_Init();	

	TCHAR file[] = { 65, 80, 80, 46, 98, 105, 110, 0 };              //APP.bin
	IFirmwareReader* sdReader = new FirmwareReaderFromSD(file);	
	IFirmwareReader* flashReader = new FirmwareReaderFromFlash(FLASH_USER_START_ADDR);
	

		
	CheckFirmwareAndCopy(sdReader, flashReader, &CopyFirmware);
	GoToUserApp();
}

void CopyFirmware(IFirmwareReader* reader, IFirmwareReader* writer)
{
	reader->Reset();
	writer->Reset();
	ReaderResult* result;
	OperationType typeRead = Word;
	uint64_t data;
	
	while (1)
	{
		free(result);
		result = reader->Read(typeRead);
		
		if (result->status == OK)
		{
			//read data value from heap
			switch (typeRead)
			{
			case DoubleWord:
				data = *(uint64_t*)result->data;
				break;
			case Word: 
				data = *(uint32_t*)result->data;
				break;
			case HalfWord: 
				data = *(uint16_t*)result->data;
				break;
			case Byte: 
				data = *(uint8_t*)result->data;
				break;
			}
			
			writer->Write(typeRead, data);
			
			continue;
		}
		
		//if tried read too much bytes
		if (result->status == TooMuchBytes)
		{
			typeRead = Byte;
			continue;
		}
		
		//if firmware is ended
		if(result->status == End)			
			break;
		
		//if throw some exception
		if (result->status == Error)
			break;
	}
}
	
void CheckFirmwareAndCopy(IFirmwareReader* source, IFirmwareReader* destination, void(*copyMethod)(IFirmwareReader*, IFirmwareReader*))
{
	ReaderResult* sourceResult = nullptr;
	ReaderResult* destinationResult = nullptr;
	
	OperationType readType = Word;
	bool isEqual = true;
	
	while (true)
	{	
		//clear memory heap
		free(sourceResult);
		free(destinationResult);
		
		//reading
		sourceResult = source->Read(readType);
		destinationResult = destination->Read(readType);
		
		
		if ((sourceResult->status == OK) && (destinationResult->status == OK))
		{
			//compare source and destination data values
			for(uint8_t i = 0 ; i < readType ; i++)
			{
				if (*(uint8_t*)sourceResult->data != *(uint8_t*)destinationResult->data)
				{
					isEqual = false;
					break;
				}
			}
			if (!isEqual)
				break;
			
			continue;
		}			
		
		//if tried read too much bytes
		if ((sourceResult->status == TooMuchBytes) || (destinationResult->status == TooMuchBytes))
		{
			//decrease reading count of bytes
			readType = Byte;
			continue;
		}
		
		
		//if destination reader returned error then copy
		if(destinationResult->status == Error)
		{
			isEqual = false;
			break;
		}
		//if source reader returned error then stoped uploading firmware
		if(sourceResult->status == Error)
		{
			break;
		}
		
		//if destination firmware is ended, but source firmware isn't ended then copy.
		//Because source firmware more than destination => firmwares aren't equals
		if(destinationResult->status == End && sourceResult->status != End)
			break;
			
		//if source readed is ended then exit from methos
		if(sourceResult->status == End)
			break;	
			
	}
	
	if (!isEqual)
		copyMethod(source, destination);
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
	PeriphDeInit();
	
	uint32_t appJumpAddress;
	void(*GoToApp)(void);

	appJumpAddress = *((volatile uint32_t*)(FLASH_USER_START_ADDR + 4));
	GoToApp = (void(*)(void))(appJumpAddress);
	__disable_irq();
	SCB->VTOR = FLASH_USER_START_ADDR;
	__set_MSP(*((volatile uint32_t*) FLASH_USER_START_ADDR));                  //stack pointer (to RAM) for USER app in this address

	
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
