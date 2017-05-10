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
BNO055_EULER_TypeDef  euler_angles;
BNO055_TEMPERATURE_TypeDef temperature;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

uint8_t MPL3115A2_Initialized = 0;



/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
void Dash7Send(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
void D7SendData(int data[],int length);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{  
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
  setI2CInterface_BNO055(&hi2c1);
  BNO055_initialize();
  setI2CInterface_MPL3115A2(&hi2c1);
  Init_Bar_MPL3115A2();
  Active_MPL3115A2();
  
  /* USER CODE BEGIN 2 */
    uint8_t opt = 0;
    char readBuf[1];
    uint8_t calibData = BNO055_read_calib_status();
    
    printf("Randomly move the BNO055 for calibration \r\n");
    
  /*Calibrate BNO055 until register value is 63, 127, 191, 255
    first two bits are system calibration bits, last 6 are for gyro, accelero and magneto
    
    63 =  00111111
    127 = 01111111
    191 = 10111111
    255 = 11111111
    
    */
    while(calibData != (63 || 127 || 191 || 255))
    {
      BNO055_change_fusion_mode(CONFIGMODE);
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      HAL_Delay(700);
      BNO055_change_fusion_mode(MODE_NDOF);
      HAL_Delay(150);
      calibData = BNO055_read_calib_status();
    }
    
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    printf("********************BNO055 calibrated******************** \r\n");
    printf("Push the blue button to send measurements over Dash7 \r\n");
    

    
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */
      
      if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
          HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
          
          //read euler angles
          BNO055_get_Euler_Angles(&euler_angles);
          printf("Heading:%+6.1f [deg], Roll:%+6.1f [deg], Pitch:%+6.1f [deg]\r\n", euler_angles.h, euler_angles.r, euler_angles.p);
          
          //read pressure
          uint32_t bar = 0;
          ReadBar_MPL3115A2_v2(&bar);
          int pressure = parseBar_MPL3115A2(bar);
          printf("Pressure: %d Pa \r\n", pressure);
          
          //send data over DASH7 (868MHz)
          int test[] = {1,2,3,4,5,6,7,8,9,10};
          D7SendData(test, (sizeof(test)/sizeof(test[0])));
            
          HAL_Delay(500);
          
          int pres[] = {pressure};
          D7SendData(pres, (sizeof(pres)/sizeof(pres[0])));
            
          HAL_Delay(500);
          
          int BNO_Angles[] = {euler_angles.h, euler_angles.r, euler_angles.p, euler_angles.h, euler_angles.r, euler_angles.p};
          D7SendData(BNO_Angles, (sizeof(BNO_Angles)/sizeof(BNO_Angles[0])));       
          
          HAL_Delay(500);
          
          HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);      
      }
      
      HAL_Delay(30);
    
  }
  /* USER CODE END 3 */

}

void getBin(int num, char *str)
{
  *(str+15) = '\0';
  int mask = 0x8000 << 1;
  while(mask >>= 1)
    *str++ = !!(mask & num) + '0';
}


static void dataToHex(int data,int dataHex[2]){
  char binstr[16];
  getBin(data, binstr);
  int len = strlen(binstr);
  
  printf("[DATA] = %d \n\r",data);
  printf("[BIN] = %s \n\r",binstr);
  
  int len1 = len/2;
  int len2 = len - len1; // Compensate for possible odd length
  char *s1 = (uint8_t*)malloc(len1+1); // one for the null terminator
  memcpy(s1, binstr, len1);
  s1[len1] = '\0';
  char *s2 = (uint8_t*)malloc(len2+1); // one for the null terminator
  memcpy(s2, binstr+len1, len2);
  s2[len2] = '\0';
  
  printf("[BIN1] = %s \n\r",s1);
  printf("[BIN2] = %s \n\n\r",s2);
  
  dataHex[0] = strtol(s1, NULL, 2);
  dataHex[1] = strtol(s2, NULL, 2);
  
  free(s1);
  free(s2);

}

void D7SendData(int data[], int length){
    
  /*  
    -----------------------------FULL COMMAND WITH DATA [0,1] -------------------
    41 54 24 44 c0 00 0e 34 af 32 d7 01 00 10 01 20 01 00 02 00 01 
    0x41 0x54 0x24 0x44 0xc0 0x00 0x0e 0x34 0xaf 0x32 0xd7 0x01 0x00 0x10 0x01 0x20 0x01 0x00 0x02 0x00 0x01
    $41 $54 $24 $44 $c0 $00 $0e $34 $af $32 $d7 $01 $00 $10 $01 $20 $01 $00 $02 $00 $01
    
            ---------------------SERIALIZE------------------------
    41 54 24 44 c0 00 0e
    
        STATIC SER       LENGTH
    [41 54 24 44 c0 00]  [0e]
    
    ++++++++++++++++ LENGTH ++++++++++++++++
    Length of bytes of the ALP COMMAND
    
    0e --> 14 --> Length
    
            ---------------------ALP COMMAND----------------------
    34 af 32 d7 01 00 10 01 20 01 00 02 00 01
    
      TAG         FORWARD         RETURN FILE DATA 
    [34 af] [32 d7 01 00 10 01] [20 01 00 02 00 01]
    
    ++++++++++++++++ TAG ++++++++++++++++
    
    ALP COMMAND: 
    34 af
    
    34 --> 52   --> Request Tag
    af --> 175  --> Tag Value (random)
    
    PARSED: Command with tag 175 (executing)
    
    ++++++++++++++++ FORWARD ++++++++++++++++
    
    ALP COMMAND:
    32 d7 01 00 10 01
    
    32 --> 50    --> Forward
    d7 --> 215   --> Interface Name (D7ASP)
    01 --> 01    --> ResponseMode.RESP_MODE_ANY
    00 --> 00    --> mant
    10 --> 16    --> ??
    01 --> 01    --> access_class
    
    PARSED:
    actions:
	action: Forward: interface-id=InterfaceType.D7ASP, configuration={'addressee': {'nls_method': <NlsMethod.NONE: 0>, 'access_class': 1, '__CLASS__': 'Addressee', 'id_type': <IdType.NOID: 1>, 'id': None}, 'qos': {'record': False, 'resp_mod': <ResponseMode.RESP_MODE_ALL: 1>, '__CLASS__': 'QoS', 'retry_mod': <RetryMode.RETRY_MODE_NO: 0>, 'stop_on_err': False}, '__CLASS__': 'Configuration', 'dorm_to': {'mant': 0, '__CLASS__': 'CT', 'exp': 0}}
    ++++++++++++++ RETURN FILE DATA ++++++++++++++++
    
    ALP COMMAND:
    20 01 00 02 00 01
    
    20 --> 32   --> Return File Data
    01 --> 01   --> File ID
    00 --> 00   --> Offset
    02 --> 02   --> Length data bytes
    
    <DATA> [0,1] LENGTH:2
    00 --> 00
    00 --> 01
    
    PARSED:
    actions:
        action: ReturnFileData: file-id=1, size=1, offset=0, length=2, data=[0, 1]
*/

   printf("Length: %d \r\n", length);
   
   if( length <=4 )
   {
     uint8_t ALPCommand[7 + ALP_LENGTH] = {  
      0x41, 0x54, 0x24, 0x44, 0xc0, 0x00, ALP_LENGTH, // SERIAL
      0x34, 0x01, //TAG
      0x32, 0xd7, 0x01, 0x00, 0x10, 0x01, //FORWARD
      0x20, 0x01, 0x00, //CommnandLine
      DASH7_DATALENGTH //Data
    };    
    
      int datacounter = 0;
      int arraycounter = 19;
      int dataHex[2];
 
      while(datacounter < length){

        dataToHex(data[datacounter],dataHex);

        ALPCommand[arraycounter] = dataHex[0];
        //printf("[SPOT0] = %d \n\n\r",arraycounter);
        //printf("[DATA%d:HEX0] = %X \n\r",datacounter,dataHex[0]);
        arraycounter++;
    
        ALPCommand[arraycounter] = dataHex[1];
        //printf("[SPOT1] = %d \n\n\r",arraycounter);
        //printf("[DATA%d:HEX1] = %X \n\n\r",datacounter,dataHex[1]);
        arraycounter++;
        datacounter++;
    
        dataHex[0] = 0;
        dataHex[1] = 0;
        
      }
   
      printf("ALP: %s \r\n", ALPCommand);
      HAL_UART_Transmit(&huart1, (uint8_t*)ALPCommand, sizeof(ALPCommand),HAL_MAX_DELAY);
   }
   
   
   else if(length <= 8)
   {
     uint8_t ALPCommand[7 + ALP_LENGTH_LARGE] = {  
      0x41, 0x54, 0x24, 0x44, 0xc0, 0x00, ALP_LENGTH_LARGE, // SERIAL
      0x34, 0x01, //TAG
      0x32, 0xd7, 0x01, 0x00, 0x10, 0x01, //FORWARD
      0x20, 0x01, 0x00, //CommnandLine
      DASH7_DATALENGTH_LARGE //Data
    };   
   
      int datacounter = 0;
      int arraycounter = 19;
      int dataHex[2];
 
      while(datacounter < length){

          dataToHex(data[datacounter],dataHex);

          ALPCommand[arraycounter] = dataHex[0];
          //printf("[SPOT0] = %d \n\n\r",arraycounter);
          //printf("[DATA%d:HEX0] = %X \n\r",datacounter,dataHex[0]);
          arraycounter++;
    
          ALPCommand[arraycounter] = dataHex[1];
          //printf("[SPOT1] = %d \n\n\r",arraycounter);
          //printf("[DATA%d:HEX1] = %X \n\n\r",datacounter,dataHex[1]);
          arraycounter++;
          datacounter++;
    
          dataHex[0] = 0;
          dataHex[1] = 0;
      }
      printf("ALP: %s \r\n", ALPCommand);
      HAL_UART_Transmit(&huart1, (uint8_t*)ALPCommand, sizeof(ALPCommand),HAL_MAX_DELAY);
   
   }
   else
   {
     printf("Data array too long. \r\n");
   } 
 
}



void Dash7Send(void)
{
  uint8_t message[] = {0x41, 0x54, 0x24, 0x44, 0xc0, 0x00, 0x0e, 0xb4, 0xaf, 0x32, 0xd7, 0x01, 0x00, 0x10, 0x01, 0x20, 0x01, 0x00, 0x02, 0x00, 0x01};
  HAL_UART_Transmit(&huart1, (uint8_t*)message, sizeof(message), HAL_MAX_DELAY);
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
