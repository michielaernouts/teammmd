#include "main.h"
#include "stm32l1xx_hal.h"

/* USER CODE BEGIN Includes */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);

//int MAG3110_WRITE_REGISTER(char reg, char val);
//void MAG3110_WRITE_REGISTER(uint8_t reg, uint8_t val);
//uint8_t MAG3110_READ_REGISTER(uint8_t reg);

uint8_t MMA8491Q_READ_REGISTER(uint8_t reg);
void MMA8491Q_WRITE_REGISTER(uint8_t reg, uint8_t val);
int MMA8491Q_Init(void);
int16_t MMA8491Q_ReadRawData_x(void);
int16_t MMA8491Q_ReadRawData_y(void);
int16_t MMA8491Q_ReadRawData_z(void);

static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);


int MAG3110_Initialized = 0;

int main(void)
{
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();

  MAG3110_Init();
 
  while (1)
  { 
    //register settings: http://www.nxp.com/assets/documents/data/en/data-sheets/MAG3110.pdf
     int16_t x = MMA8491Q_ReadRawData_x();
     int16_t y = MMA8491Q_ReadRawData_y();
     int16_t z = MMA8491Q_ReadRawData_z();
     
     printf("x: : %d , y: : %d , z: : %d  \r \n", x, y, z);
     
     HAL_Delay(10);

  }
}


// pin hoog zetten en regs lezen

uint8_t MMA8491Q_READ_REGISTER(uint8_t reg) {
	//char* ch = malloc(1);
	//bcm2835_i2c_read_register_rs(&reg, ch, 1);
	//return ch[0];
        uint8_t status3;
        //0X0E = The 7-bit I2C slave address 
        int status2 = HAL_I2C_Mem_Read(&hi2c1, 0x55 << 1, reg, I2C_MEMADD_SIZE_8BIT, &status3, 1, HAL_MAX_DELAY);
        return status3;
}


void MMA8491Q(uint8_t reg, uint8_t val) {
	//bcm2835_i2c_setSlaveAddress(14);
	//bcm2835_i2c_write(&x, 2);

        uint8_t arr[] = {reg, val};
        //0X0E = The 7-bit I2C slave address 
        int status=HAL_I2C_Master_Transmit(&hi2c1, 0x55 << 1, arr, 2, HAL_MAX_DELAY);
}


int MMA8491Q_Init() {
	char v = 0;
       
        MAG3110_WRITE_REGISTER(16, 2);  /*CTRL_REG1 0 schrijven*/ 
	v = MAG3110_READ_REGISTER(7); /*WHO_AM_I lezen default value = 0xc4 */
        //printf("v = %c\n",v);
       
         if (v == 0xc4) {  //0xc4 = default value of reg 7
		MAG3110_WRITE_REGISTER(17, 128);  //CTRL_REG_2 schrijven AUTO_MRST_EN = 1 (0x80 = 128)
		MAG3110_Initialized = 1;
	}
	return MAG3110_Initialized;   
}



int16_t MAG3110_ReadRawData_x() {
        char r = 0;
	int16_t a = 0;
	if (MAG3110_Initialized == 1) {
		a = MAG3110_READ_REGISTER(1);
		a = MAG3110_READ_REGISTER(2);
		MAG3110_WRITE_REGISTER(17, 128); //(0x11, 0x80 ) = CTRL_REG2 Automatic magnetic sensor resets enabled
		MAG3110_WRITE_REGISTER(16, 2); //(0x10, 0x02 ) = CTRL_REG1 = Fast read mode 
		do {
			r = MAG3110_READ_REGISTER(0);
		} while (!(r & 1));
		a = MAG3110_READ_REGISTER(1);
		a = a << 8;
		a = a + MAG3110_READ_REGISTER(2);
	}	
	return a;
}


void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
	  GPIO_InitTypeDef GPIO_InitStruct;

	  /* Peripheral clock enable */
	  __HAL_RCC_I2C1_CLK_ENABLE();

	  hi2c1.Instance = I2C1;
	  hi2c1.Init.ClockSpeed = 100000;
	  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	  hi2c1.Init.OwnAddress1 = 0;
	  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	  hi2c1.Init.OwnAddress2 = 0;
	  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	  //HAL_I2C_Init(&hi2c1);

	  GPIO_InitStruct.Pin =GPIO_PIN_6|GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

}

/* USER CODE BEGIN 4 */

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

PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
