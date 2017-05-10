#include "main.h"
#include "stm32l1xx_hal.h"

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
/* USER CODE BEGIN PV */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

void CHIP_SELECT_WAKE_UP(void);
/*
 * left shift of register: if we read or write, HAL adds a 1 or 0 to the end of the register
 * our HAL doesn't shift automatically like an arduino, so we need to shift it ourselves.
 */
void ADXL345_WRITE_REGISTER(uint8_t reg, uint8_t value) {
    uint8_t pData[] = { reg, value };
    int status = HAL_I2C_Master_Transmit(&hi2c1, 0x53 << 1, pData, 2,
    HAL_MAX_DELAY);
}
uint8_t ADXL345_READ_REGISTER(uint8_t reg) {
    uint8_t buf;
    uint8_t status = HAL_I2C_Mem_Read(&hi2c1, 0x53 << 1, reg, I2C_MEMADD_SIZE_8BIT,
            &buf, 1, HAL_MAX_DELAY);
   // char str[80];
  //  sprintf(str, "status = %d\r\n", status);
   // HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), HAL_MAX_DELAY);
    return buf;
}



void CHIP_SELECT_WAKE_UP()
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
  HAL_Delay(10);
}

int main(void) {

    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    char str2[80];
      
    /*0xE5 (229) default, when CS connected to GPIO becomes 0xDF (220) */
    uint8_t devid = ADXL345_READ_REGISTER(0x00); 
    
    sprintf(str2, "DEV ID = %d\r\n", devid);
    HAL_UART_Transmit(&huart2, (uint8_t*) str2, strlen(str2), HAL_MAX_DELAY);
    
    ADXL345_WRITE_REGISTER(0x2D, 0x00); //Go into standby mode to configure the device.
    
    
    //0x08 = +/-2g
    //0x09 = +/-4g
    //0x0A = +/-8g
    //0x0B = +/-16g
    
    ADXL345_WRITE_REGISTER(0x31, 0x08); //Full resolution, +/-16g, 4mg/LSB.
    ADXL345_WRITE_REGISTER(0x2C, 0x0F); //3.2kHz data rate.No low power mode
    ADXL345_WRITE_REGISTER(0x2D, 0x80); //Measurement mode.
 
    CHIP_SELECT_WAKE_UP();
    
    while (1) {
        
        uint8_t x1 = ADXL345_READ_REGISTER(0x32);
        uint8_t x2 = ADXL345_READ_REGISTER(0x33);
        
        uint8_t y1 = ADXL345_READ_REGISTER(0x34);
        uint8_t y2 = ADXL345_READ_REGISTER(0x35);
        
        uint8_t z1 = ADXL345_READ_REGISTER(0x36);
        uint8_t z2 = ADXL345_READ_REGISTER(0x37);
       /* uint16_t out_x = x1 << 8 | x2;
        uint16_t out_y = y1 << 8 | y2;
        uint16_t out_z = z1 << 8 | z2;*/
        
        uint16_t out_x = x1;
        uint16_t out_y = y1;
        uint16_t out_z = z1;
        
        char str3[80];
        sprintf(str3, " out_x = %d | out_y = %d | out_z = %d\r\n", out_x, out_y, out_z);
        HAL_UART_Transmit(&huart2, (uint8_t*) str3, strlen(str3), HAL_MAX_DELAY);
       
        HAL_Delay(100); 
    }

}


/** System Clock Configuration
 */
void SystemClock_Config(void) {
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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /**Initializes the CPU, AHB and APB busses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
    /**Configure the Systick interrupt time
     */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    /**Configure the Systick
     */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
/* I2C1 init function */
static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 50000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}
/* USART2 init function */
static void MX_USART2_UART_Init(void) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
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
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    /*Configure GPIO pin : PB8 */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    
    
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler */
    /* User can add his own implementation to report the HAL 100000error return state */
    while (1) {
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