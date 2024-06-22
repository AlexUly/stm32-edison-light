/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pca9535.h"
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart4;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */
int MODE = 0; 	     //Режим работы
int MODE_NUM = 4;
char commandBuf[32]; //Буфер обработки комманды
char outputBuf[64];  //Буфер ответа на комманды
int msgLen = 0;		 //Длина комманды
int PIN = 0;
int DELAY = 1000;	 //Задержка цикла
int TIMESTAMP;
int SPEED = 100;	 //Скорость мерцания диодов
int DELAY_DEFAULT_MODE_1 = 500;
int DELAY_DEFAULT_MODE_2 = 1000;
int DELAY_DEFAULT_MODE_3 = 1500;
int SPEED_DEFAULT_MODE_1 = 100;
int SPEED_DEFAULT_MODE_2 = 200;
int SPEED_DEFAULT_MODE_3 = 400;
int DIRECTION = 1; // Направление движения диодов, положительное или отрицательное
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_UART4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void setModeParam(int _MODE){
	if (_MODE = MODE){
		MODE = 0;
		return;
	}
	switch (_MODE){
	case 1:
		MODE = 1;
		DELAY = DELAY_DEFAULT_MODE_1;
		SPEED = SPEED_DEFAULT_MODE_1;
		DIRECTION = 1;
		break;
	case 2:
		MODE = 2;
		DELAY = DELAY_DEFAULT_MODE_2;
		SPEED = SPEED_DEFAULT_MODE_2;
		DIRECTION = 1;
		break;
	case 3:
		MODE = 3;
		DELAY = DELAY_DEFAULT_MODE_3;
		SPEED = SPEED_DEFAULT_MODE_3;
		DIRECTION = 1;
		break;
	}
	return;
}

void button_1_IRQ_Callback(){
	setMode(1);
}
void button_2_IRQ_Callback(){
	setMode(2);
}
void button_3_IRQ_Callback(){
	setMode(3);
}

int UART_IRQ_callback() {
	 if (!__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE)) {
		 return 0;
	 }
	 commandBuf[msgLen % INPUT_BUF_SIZE] = huart4.Instance->RDR;
	 msgLen++;
	 return 0;
}

//Выствляем все выходы в 0
void clearOutput(){
	PCA9535_WriteClearAll(hi2c1, PCA9555_ADR20);
	PCA9535_WriteClearAll(hi2c1, PCA9555_ADR21);
}
//Выставляем все выходы в 1
void setOutput(){
	PCA9535_WriteSetAll(hi2c1, PCA9555_ADR20);
	PCA9535_WriteSetAll(hi2c1, PCA9555_ADR21);
}
//Режим комета
void mode_1(){
	HAL_Delay(1000 /SPEED_DEFAULT_MODE_1);
	PIN += DIRECTION;
	if(PIN > 33 || PIN < 0){
		clearOutput();
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, RESET);
		if(DIRECTION > 0)
			PIN = 0;
		else
			PIN = 33;
		return;
	}
	if(PIN < 15){
	     PCA9535_Write(hi2c1, PIN, SET , PCA9555_ADR20);
	     return;
	}
	if(PIN < 32){
	     PCA9535_Write(hi2c1, PIN - 16, SET , PCA9555_ADR21);
	     return;
	}
	if(PIN == 33){
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, SET);
		return;
    }
}
//Режим падение
void mode_2(){
	HAL_Delay(1000 /SPEED_DEFAULT_MODE_2);
	PIN -= DIRECTION;
	if(PIN > 33 || PIN < 0){
		clearOutput();
		if(DIRECTION > 0)
			PIN = 33;
		else PIN = 0;
		return;
	}

	if(PIN == 33){
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, SET);
		return;
	}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, RESET);
	if(PIN < 32){
		 clearOutput();
	     PCA9535_Write(hi2c1, PIN - 16, SET , PCA9555_ADR21);
	     return;
	}
	if(PIN < 15){
		 clearOutput();
	     PCA9535_Write(hi2c1, PIN, SET , PCA9555_ADR20);
	     return;
	}
}
//Режим салют
void mode_3(){
	HAL_Delay(1000 / SPEED_DEFAULT_MODE_3);
	PIN += DIRECTION;
	if(PIN > 32 || PIN < 0){
		setOutput();
		if(DIRECTION > 0)
			PIN = 0;
		else
			PIN = 32;
		return;
	 }
	if(PIN < 15){
		 setOutput();
	     PCA9535_Write(hi2c1, PIN, RESET , PCA9555_ADR20);
	     return;
	}
	if(PIN < 32){
		 setOutput();
	     PCA9535_Write(hi2c1, PIN - 16, RESET , PCA9555_ADR21);
	     return;
	}
}

void setDelay(int _delay){
	DELAY = abs(_delay) % 3000;
}

void setMode(int _mode){
	_mode = abs(_mode) % MODE_NUM;
	setModeParam(_mode);
}

void setSpeed(int _speed){
	SPEED = abs(_speed) % 1000;
}

void setDirection(int _dir){
	(_dir > 0) ? (DIRECTION = 1) : (DIRECTION = -1);
}
//Обработка AT-Комманд
void commandProcess(){
	int n = 0;
	int cmd_len = strlen(commandBuf);
	sscanf(commandBuf, "AT+TURNOFF%n", &n);
	if (!strncmp(commandBuf, "AT+TURNOFF", 10)){
		strcpy(outputBuf, "OK\n");
		MODE = 0;
		return;
	}
	if (!strncmp(commandBuf, "AT+SETMODE:", 11)){
		int tempMode = -1;
		sscanf(commandBuf, "AT+SETMODE:%d", &tempMode);
		setMode(tempMode);
		strcpy(outputBuf, "OK");
		return;
	}
	if (!strncmp(commandBuf, "AT+SETDELAY:", 12)){
		int tempDelay = 100;
		sscanf(commandBuf, "AT+SETDELAY:%d", &tempDelay);
		setDelay(tempDelay);
		strcpy(outputBuf, "OK");
		return;
	}
	if (!strncmp(commandBuf, "AT+SETSPEED:", 12)){
		int tempSpeed = 100;
		sscanf(commandBuf, "AT+SETSPEED:%d", &tempSpeed);
		setSpeed(tempSpeed);
		strcpy(outputBuf, "OK");
		return;
	}
	if (!strncmp(commandBuf, "AT+SETDIRECTION:", 16)){
		int tempDir = 100;
		sscanf(commandBuf, "AT+SETDIRECTION:%d", &tempDir);
		setDirection(tempDir);
		strcpy(outputBuf, "OK");
		return;
	}
	if (!strncmp(commandBuf, "AT+GETMODE", 10)){
		if(!MODE){
			strcpy(outputBuf, "OFF");
			return;
		}
		sprintf(outputBuf, "ON MODE:%d SPEED:%d DELAY:%d DIRECTION: %d", MODE, SPEED, DELAY, DIRECTION);
		return;
	}
	strcpy(outputBuf, "ERROR");
}
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
  MX_I2C1_Init();
  MX_USB_PCD_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
  //Включаем прерывания по RXNE
   __HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
   TIMESTAMP = HAL_GetTick();
   //Настраеваем порты PC9535 с адрессами 0x20 и 0x21 на выход
   PC9535_init(hi2c1, PCA9555_ADR20);
   PC9535_init(hi2c1, PCA9555_ADR21);
   //Выставляем все выходы в 0
   clearOutput();
   HAL_UART_Transmit(&huart4, "Start", 5, HAL_MAX_DELAY);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   while (1) {
	   HAL_Delay(1000);
	   	   if(msgLen){
	   	   	   commandProcess();
	   	   	   HAL_UART_Transmit(&huart4, outputBuf, strlen(outputBuf), 1000);
	   	 	   HAL_UART_Transmit(&huart4, "\n", 1, 1000);
	   	   	   msgLen = 0;
	   	   	   memset(commandBuf, 0 , 32);
	   	   	   memset(outputBuf, 0 , 64);
	   	   }
	if(HAL_GetTick() - TIMESTAMP > DELAY){
		TIMESTAMP = HAL_GetTick();
	}
	else
		continue;

	switch(MODE){
		case 1:
			mode_1();
			break;
		case 2:
			mode_2();
			break;
		case 3:
			mode_3();
			break;
		default:
			break;
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
  huart4.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);

  /*Configure GPIO pins : CS_I2C_SPI_Pin LD4_Pin LD3_Pin LD5_Pin
                           LD7_Pin LD9_Pin LD10_Pin LD8_Pin
                           LD6_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : MEMS_INT3_Pin MEMS_INT4_Pin MEMS_INT1_Pin */
  GPIO_InitStruct.Pin = MEMS_INT3_Pin|MEMS_INT4_Pin|MEMS_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC1 PC2 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_SCK_Pin SPI1_MISO_Pin SPI1_MISOA7_Pin */
  GPIO_InitStruct.Pin = SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MISOA7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
