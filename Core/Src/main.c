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
#include "cmsis_os.h"
#include "event_groups.h"
#include <stdbool.h>

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DELAY1_MS 				300
#define TASK1_BITS				(1 << 0)
#define TASK2_BITS				(1 << 1)
#define TASK3_BITS				(1 << 2)
#define STACK_SIZE				128 * 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define PA5_on() 				GPIOA->BSRR = (1 << 5);
#define PA5_off() 				GPIOA->BRR = (1 << 5);
#define LED_status_toggle() 	led_status = !led_status;
#define PC13_is_on() 			(GPIOC->IDR & GPIO_PIN_13) == 0
#define PA5_handle() \
	if ( led_status ) { \
		PA5_on(); \
	} else { \
		PA5_off(); \
	}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
	.name = "defaultTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t firstTaskHandle;
const osThreadAttr_t firstTask_attributes = {
	.name = "firstTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t secondTaskHandle;
const osThreadAttr_t secondTask_attributes = {
	.name = "secondTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t thirdTaskHandle;
const osThreadAttr_t thirdTask_attributes = {
	.name = "thirdTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t fourthTaskHandle;
const osThreadAttr_t fourthTask_attributes = {
	.name = "fourthTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

/* USER CODE BEGIN PV */
EventGroupHandle_t group;
bool led_status = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* STM32 functions */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* Functions */
void debug_blink (int amount);

/* Task functions */
void StartDefaultTask(void *argument);
void FirstTask (void *argument);
void SecondTask (void *argument);
void ThirdTask (void *argument);
void FourthTask (void *argument);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();

	/* Init scheduler */
	osKernelInitialize();

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1);
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 10;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : PA5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void debug_blink (int amount) {
	for ( int i = 0; i < amount * 2; i++ ) {
		LED_status_toggle();
		PA5_handle();
		vTaskDelay(amount * 10);
	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask (void *argument) {
	group = xEventGroupCreate();

	firstTaskHandle = osThreadNew(FirstTask, NULL, &firstTask_attributes);

	secondTaskHandle = osThreadNew(SecondTask, NULL, &secondTask_attributes);

	thirdTaskHandle = osThreadNew(ThirdTask, NULL, &thirdTask_attributes);

	fourthTaskHandle = osThreadNew(FourthTask, NULL, &fourthTask_attributes);

	while (1);
}

void FirstTask (void *argument) {
	vTaskDelay(DELAY1_MS);

	xEventGroupSetBits(group, TASK1_BITS);

	while (1);
}

void SecondTask (void *argument) {
	xEventGroupWaitBits(
		group,
		TASK1_BITS,
		pdFALSE,
		pdTRUE,
		portMAX_DELAY
	);

	xEventGroupSetBits(group, TASK2_BITS);

	while (1);
}

void ThirdTask (void *argument) {
	xEventGroupWaitBits(
		group,
		TASK1_BITS | TASK2_BITS,
		pdFALSE,
		pdTRUE,
		portMAX_DELAY
	);

	while ( ! PC13_is_on() );

	xEventGroupSetBits(group, TASK3_BITS);

	while (1);
}

void FourthTask (void *argument) {
	xEventGroupWaitBits(
		group,
		TASK1_BITS | TASK2_BITS | TASK3_BITS,
		pdFALSE,
		pdTRUE,
		portMAX_DELAY
	);

	debug_blink(25);

	led_status = true;
	PA5_handle();

	while (1);
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
