/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l1xx_hal.h"
#include "BNO055.h"
#include "MPL3115A2.h"


/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

//SMALL SIZE PACKAGE
#define DASH7_ARRAYLENGTH                       4
#define DASH7_DATALENGTH                        DASH7_ARRAYLENGTH * 2
#define ALP_LENGTH                              12 + DASH7_DATALENGTH
#define STARTUPNFC                      20 //used to be significant
#define STARTUPDASH7                    3000 //1250 would probably also work
#define SENDTIMEDASH7					500 //
#define LOOKFORCARDTIME					5000
#define MAXTRIES						10

//LARGE SIZE PACKAGE
#define DASH7_ARRAYLENGTH_LARGE                 8
#define DASH7_DATALENGTH_LARGE                  DASH7_ARRAYLENGTH_LARGE * 2
#define ALP_LENGTH_LARGE                        12 + DASH7_DATALENGTH_LARGE
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0])) // macro to get length of array
BNO055_EULER_TypeDef  euler_angles;
BNO055_MAG_TypeDef mag;

uint8_t timerexpired = 0;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);		
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
void SleepMode(void);
void toggleRed(uint8_t time);
void toggleGreen(uint8_t time);
void toggleBlue(uint8_t time);
void toggleWhite(uint8_t time);
uint8_t checkCalib();
void calibrateBNO();
void D7SendMessage(uint8_t data[], int length);
void bootDASH7();
void shutDownDASH7();

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */

	//init MPL for temperature and pressure
	setI2CInterface_MPL3115A2(&hi2c1);
	Init_Bar_int_MPL3115A2();
	Active_MPL3115A2();
	
	
	//setI2CInterface_BNO055(&hi2c1);
	//HAL_Delay(3000);//startup BNO
	//BNO055_initialize();
	HAL_Delay(1000);//startup BNO
  
   /* USER CODE END 2 */
	toggleWhite(100);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */
	/* USER CODE END WHILE */
	  /*
	if(BNO055_get_Ready()!=15)
		{
			while(1)
			{
				toggleWhite(10);
				HAL_Delay(3000);
			}
		}
*/
		//calibrateBNO();
		
		//Enable and start timer interrupt
		MX_TIM2_Init();
		__HAL_RCC_TIM2_CLK_ENABLE(); //Enable the TIM2 peripheral
		HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0); //Enable the peripheral IRQ
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
		HAL_TIM_Base_Init(&htim2); //Configure the timer
		__HAL_TIM_CLEAR_FLAG(&htim2, TIM_SR_UIF);
		HAL_TIM_Base_Start_IT(&htim2); //Start the timer

		//BNO055_Suspend();

		while (1)
  		{
  			if(timerexpired==1)
  			{
				toggleBlue(50);
				bootDASH7();
				//HAL_Delay(500);

				//get pressure
				uint32_t barData;
				ReadBar_MPL3115A2_v2(&barData);
				uint8_t bar[3];	
				bar[0] = (barData >> 16) & 0xFF;
				bar[1] = (barData >> 8) & 0xFF;
				bar[2] = (barData >> 0) & 0xFF;
				
				//get temperature	
				uint16_t temperature = Read_Temp_MPL3115A();
				uint8_t temp[2];
				temp[0] = (temperature >> 8) & 0xFF;
				temp[1] = (temperature >> 0) & 0xFF;
				
				//send message
				uint8_t message[] = {21, bar[0], bar[1], bar[2], temp[0], temp[1]};
				D7SendMessage(message, NELEMS(message));
				timerexpired = 0;
				shutDownDASH7();
				SleepMode();
  			}
  		}

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 2000; //prescale to ms
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;	//period of 10s
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, VNFC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Button_Pin */
  GPIO_InitStruct.Pin = Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LEDG_Pin LEDR_Pin LEDB_Pin RSTNFC_RBNO_Pin 
                           VNFC_Pin */
  GPIO_InitStruct.Pin = LEDG_Pin|LEDR_Pin|LEDB_Pin|RSTNFC_RBNO_Pin 
                          |VNFC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOA, LEDG_Pin | LEDR_Pin | LEDB_Pin|RSTNFC_RBNO_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, VNFC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : IPIR_IBAR_Pin */
  GPIO_InitStruct.Pin = IPIR_IBAR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IPIR_IBAR_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);


  /*Configure GPIO pin : INFC_IBNO_Pin */
  GPIO_InitStruct.Pin = INFC_IBNO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INFC_IBNO_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void D7SendMessage(uint8_t data[], int length)
{
  if( length <=8 )
   {
     uint8_t ALPCommand[7 + ALP_LENGTH] = {
      0x41, 0x54, 0x24, 0x44, 0xc0, 0x00, ALP_LENGTH, // SERIAL
      0x34, 0x01, //TAG
      0x32, 0xd7, 0x01, 0x00, 0x10, 0x01, //FORWARD
      0x20, 0x01, 0x00, //CommnandLine
      DASH7_DATALENGTH //Data
    };

      int datacounter = 0;;

      while(datacounter < length){
        ALPCommand[19+datacounter] = data[datacounter];
        //printf("Dash7 payload: %d \r\n", data[datacounter]);
        datacounter++;
      }
      HAL_UART_Transmit(&huart1, (uint8_t*)ALPCommand, sizeof(ALPCommand),HAL_MAX_DELAY);
      //printf("*** Dash7 Message sent *** \r\n");
   }


   else if(length <= 16)
   {
     uint8_t ALPCommand[7 + ALP_LENGTH_LARGE] = {
      0x41, 0x54, 0x24, 0x44, 0xc0, 0x00, ALP_LENGTH_LARGE, // SERIAL
      0x34, 0x01, //TAG
      0x32, 0xd7, 0x01, 0x00, 0x10, 0x01, //FORWARD
      0x20, 0x01, 0x00, //CommnandLine
      DASH7_DATALENGTH_LARGE //Data
    };

      int datacounter = 0;

      while(datacounter < length){
          ALPCommand[19+datacounter] = data[datacounter];
          //printf("Dash7 payload: %d \r\n", data[datacounter]);
          datacounter++;
      }

      HAL_UART_Transmit(&huart1, (uint8_t*)ALPCommand, sizeof(ALPCommand),HAL_MAX_DELAY);
     // printf("*** Dash7 Message sent *** \r\n");

   }
   else
   {
     //printf("Data array too long. \r\n");
   }

}

/* USER CODE BEGIN 4 */
void bootDASH7()
{
	HAL_GPIO_WritePin(GPIOA, RSTNFC_RBNO_Pin, GPIO_PIN_SET);
	MX_USART1_UART_Init();
	HAL_Delay(STARTUPDASH7);
}
void shutDownDASH7()
{
	HAL_Delay(SENDTIMEDASH7);
	HAL_UART_DeInit(&huart1);
	HAL_GPIO_WritePin(GPIOA, RSTNFC_RBNO_Pin, GPIO_PIN_RESET);
}

void toggleRed(uint8_t time)
{
	if(time==0)
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); //RED LED
	else
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); //RED LED
		HAL_Delay(time);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); //RED LED
	}
}
void toggleGreen(uint8_t time)
{
	if(time==0)
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //RED LED
	else
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //RED LED
		HAL_Delay(time);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //RED LED
	}
}
void toggleBlue(uint8_t time)
{
	if(time==0)
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //RED LED
	else
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //RED LED
		HAL_Delay(time);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //RED LED
	}
}
void toggleWhite(uint8_t time)
{
	if(time==0)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //RED LED
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); //GREEN LED
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //BlUE LED
	}
	else
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //RED LED
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); //GREEN LED
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //BlUE LED
		HAL_Delay(time);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //RED LED
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); //GREEN LED
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //BlUE LED
	}
}

uint8_t checkCalib()
{
	uint8_t calibData = BNO055_read_calib_status();
	 if((calibData == 51) ||(calibData== 55) ||(calibData== 59) ||(calibData== 63) ||(calibData== 127) ||(calibData== 191) ||(calibData== 255))
		 return 1;
	 else
		 return 0;
}
void calibrateBNO()
{
	toggleRed(0);
	//BNO055_change_fusion_mode(CONFIGMODE);
	 BNO055_change_fusion_mode(MODE_NDOF);

	while(checkCalib()==0)
	{

	  HAL_Delay(5000);
	  toggleBlue(10);
	}

	toggleRed(0);
}

void SleepMode(void)
{
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	HAL_SuspendTick();
	__HAL_RCC_PWR_CLK_ENABLE();
	 //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI); //LOWpowerstopMode
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
	HAL_ResumeTick();
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
	//__HAL_GPIO_EXTI_CLEAR_IT(11);
}


/*void TIM2_IRQHandler(void)
{
      //if more than 1 timer is used with interrupts enabled, we have to manually check which interrupt flag has been set  
     HAL_TIM_IRQHandler(&htim2);
}*/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if(htim->Instance==TIM2)
    {
		timerexpired=1;
		__HAL_TIM_CLEAR_FLAG(&htim2, TIM_SR_UIF);
		HAL_TIM_Base_Start_IT(&htim2); //Start the timer
    }
    
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	  if(GPIO_Pin == GPIO_PIN_11){
		//  flags = flags | 0b00000001;
	  }
 }

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
