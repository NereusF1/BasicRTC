/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */
uint32_t tempBBRAMStatus; //0x32F2 if RTC Set in BBRAM
uint8_t RTCDateTimeInitAtStartup;

RTC_DateTypeDef sdateget;
RTC_TimeTypeDef stimeget;
RTC_DateTypeDef sdateset;
RTC_TimeTypeDef stimeset;

uint8_t getCurrentDateTime;
uint8_t setCurrentDateTime;
uint16_t OnCountTarget;
uint16_t OffCountTarget;
uint8_t getOnOffTarget;
uint8_t setOnOffTarget; // 0=> no changes pending 1=> changes pending

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
uint32_t pack16bit(uint16_t c0, uint16_t c1);
uint16_t unpack_c0_u16bit(uint32_t p);
uint16_t unpack_c1_u16bit(uint32_t p);

uint32_t leap (uint32_t year);
uint32_t zeller (uint32_t year, uint32_t month, uint32_t day);
uint32_t dow (uint32_t year, uint32_t month, uint32_t day);

void RTC_CalendarConfig(RTC_DateTypeDef *, RTC_TimeTypeDef *);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(getCurrentDateTime!=0)
	  {
		/* Get the RTC current Time */
		HAL_RTC_GetTime(&hrtc, &stimeget, RTC_FORMAT_BIN);
		/* Get the RTC current Date */
		HAL_RTC_GetDate(&hrtc, &sdateget, RTC_FORMAT_BIN);
		getCurrentDateTime=0;
	  }

	  if(setCurrentDateTime!=0)
	  {
		  RTC_CalendarConfig(&sdateset, &stimeset); // may need to consider a +1s to be more accurate..
		  setCurrentDateTime=0;
	  }

	if(getOnOffTarget!=0)
	{
		uint32_t tempOnOffTarget;
		tempOnOffTarget = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR8); //<<<<<<8
		OnCountTarget = unpack_c1_u16bit(tempOnOffTarget); //("C1",C0)
		OffCountTarget = unpack_c0_u16bit(tempOnOffTarget); //("C1,"C0)"
		getOnOffTarget=0;
	}

	if (setOnOffTarget != 0) {
		uint32_t tempCountPack;
		tempCountPack = pack16bit(OnCountTarget, OffCountTarget); // (c1,c0)
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR8, tempCountPack); //Alarm time  //<<<<<<<<8
		setOnOffTarget = 0;
	}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON_RTC_ONLY;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 24;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_PrivilegeStateTypeDef privilegeState = {0};
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
  privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
  privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
  privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
  if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  //nff26 start - working code but we may lose sub-seconds (upto ~1s) by calling HAL_RTC_Init()
  //.. there is now a fix in HAL_RTC_Init() that checks for non-zero year and skips the initialization and losses...
  tempBBRAMStatus = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
  if (tempBBRAMStatus != 0x32F2) //if RTC & BBRAM is invalid then initialize to 01-01-2000 00:00:00
	{
  //nff26 -- end

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,0x32F2); //writes data to RTC Backup register indicating RTC configuration is done
  RTCDateTimeInitAtStartup=1;
	}
  else
  {
	  HAL_PWR_EnableBkUpAccess();  //unable to figure out what is being done? -- enabling write access to BKUP Regs
	  /*Peripheral clock enable*/
	  __HAL_RCC_RTC_ENABLE();  //Apparently trying to set RCC_BDCR(reg)->RTCEN(bit)=1 -- enables RTC Ticking...
	  HAL_NVIC_SetPriority(RTC_IRQn,1,0);
	  HAL_NVIC_EnableIRQ(RTC_IRQn);
  }

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
uint32_t pack16bit(uint16_t c1, uint16_t c0)  // (c1,c0)
{
    return (((uint32_t)c1 << 16) | (uint32_t)c0);
}

uint16_t unpack_c1_u16bit(uint32_t p)
{
    return p >> 16;
}

uint16_t unpack_c0_u16bit(uint32_t p)
{
    return p;
}

//Day of Week Calc - Start
/* Returns the number of days to the start of the specified year, taking leap
 * years into account, but not the shift from the Julian calendar to the
 * Gregorian calendar. Instead, it is as though the Gregorian calendar is
 * extrapolated back in time to a hypothetical "year zero".
 */
uint32_t leap (uint32_t year)
{
  return year*365 + (year/4) - (year/100) + (year/400);
}

/* Returns a number representing the number of days since March 1 in the
 * hypothetical year 0, not counting the change from the Julian calendar
 * to the Gregorian calendar that occured in the 16th century. This
 * algorithm is loosely based on a function known as "Zeller's Congruence".
 * This number MOD 7 gives the day of week, where 0 = Monday and 6 = Sunday.
 */
uint32_t zeller (uint32_t year, uint32_t month, uint32_t day)
{
  year += ((month+9)/12) - 1;
  month = (month+9) % 12;
  return leap (year) + month*30 + ((6*month+5)/10) + day + 1;
}

/* Returns the day of week (1=Monday, 7=Sunday) for a given date.
 */
uint32_t dow (uint32_t year, uint32_t month, uint32_t day)
{
  return (zeller (year, month, day) % 7) + 1;
}
//Day of Week Calc - End

//RTC Start by Reference
void RTC_CalendarConfig(RTC_DateTypeDef *sdatestructure, RTC_TimeTypeDef *stimestructure)
{

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure->WeekDay=(uint8_t)dow(sdatestructure->Year, sdatestructure->Month, sdatestructure->Date);

  if(HAL_RTC_SetDate(&hrtc,sdatestructure,RTC_FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  //stimestructure->SubSeconds =0; // may not be required for the SetTime()//commented for F105
  //stimestructure->SecondFraction =255; // may not be required for the SetTime()//commented for F105
  //stimestructure.TimeFormat = stimeset.TimeFormat; //24h

  if (HAL_RTC_SetTime(&hrtc, stimestructure, RTC_FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
