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

//needs to be usigned, because of two's complement number in register
uint16_t Accelero_X_Value = 0;
uint16_t Accelero_Y_Value = 0;
uint16_t Accelero_Z_Value = 0;
void EN_PIN_LVL_CHANGE(void);

/*
 * left shift of register: if we read or write, HAL adds a 1 or 0 to the end of the register
 * our HAL doesn't shift automatically like an arduino, so we need to shift it ourselves.
 */
uint8_t MMA8491Q_READ_REGISTER(uint8_t reg) {
        uint8_t buff;
        // 7-bit slave address = 0x55 
        int status2 = HAL_I2C_Mem_Read(&hi2c1, 0x55 << 1, reg, I2C_MEMADD_SIZE_8BIT, &buff, 1, HAL_MAX_DELAY);
        return buff;
}


void MMA8491Q_WRITE_REGISTER(uint8_t reg, uint8_t val) {
        uint8_t arr[] = {reg, val};
         /* 7-bit slave address = 0x55*/
        int status=HAL_I2C_Master_Transmit(&hi2c1, 0x55 << 1, arr, 2, HAL_MAX_DELAY);
}

/*
 * Changes the level for the EN-pin for the MMA8491 from low to high and visa versa when called
 */
void EN_PIN_LVL_CHANGE() {
  
    GPIO_PinState status = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
    if (status == GPIO_PIN_RESET) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
    } else if (status == GPIO_PIN_SET) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    }
}
uint8_t getRegisterInfo() {
    //depending on the value of the 0x00 register, data changes
    uint8_t status = MMA8491Q_READ_REGISTER(0x00);
    
    /*char str[80];
    sprintf(str, "status = %d\r\n", status);
    HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), HAL_MAX_DELAY);
    
    */
     if (status == 0x08) { //new set of data is ready
     return 1;
     } else if (status == 0x01) { //new x-data ready
     return 2;
     } else if (status == 0x02) { //new y-data ready
     return 3;
     } else if (status == 0x04) { //new z-data ready
     return 4;
     }
    return 1;
}

int main(void) {

    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();
    /* Initialize all configured peripherals */
    //HAL_MspInit();
    MX_GPIO_Init();
    //HAL_I2C_MspInit(&hi2c1);
    //HAL_UART_MspInit(&huart2);
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    
    EN_PIN_LVL_CHANGE();
       
    uint8_t b, c, d, e, f, g = 0;
    while (1) {
 
        //We read register to see what data has been read
        uint8_t reg = getRegisterInfo();
        switch (reg) {
        case 1: //new set of data is ready
            //read x-value
            Accelero_X_Value = MMA8491Q_READ_REGISTER(0x01);
            Accelero_X_Value <<= 8;
            b = MMA8491Q_READ_REGISTER(0x02);
            Accelero_X_Value = Accelero_X_Value | b;
            //read y-value
            Accelero_Y_Value = MMA8491Q_READ_REGISTER(0x03);
            Accelero_Y_Value <<= 8;
            c = MMA8491Q_READ_REGISTER(0x04);
            Accelero_Y_Value = Accelero_Y_Value | c;
            //read z-value
            Accelero_Z_Value = MMA8491Q_READ_REGISTER(0x05);
            Accelero_Z_Value <<= 8;
            d = MMA8491Q_READ_REGISTER(0x06);
            Accelero_Z_Value = Accelero_Z_Value | d;
           break;
        /*case 2: //new x-data ready
            Accelero_X_Value = MMA8491Q_READ_REGISTER(0x01);
            Accelero_X_Value <<= 8;
            e = MMA8491Q_READ_REGISTER(0x02);
            Accelero_X_Value = Accelero_X_Value | e;
            break;
        case 3: //new y-data ready
            Accelero_Y_Value = MMA8491Q_READ_REGISTER(0x03);
            Accelero_Y_Value <<= 8;
            f = MMA8491Q_READ_REGISTER(0x04);
            Accelero_Y_Value = Accelero_Y_Value | f;
            break;
        case 4: //new z-data ready
            Accelero_Z_Value = MMA8491Q_READ_REGISTER(0x05);
            Accelero_Z_Value <<= 8;
            g = MMA8491Q_READ_REGISTER(0x06);
            Accelero_Z_Value = Accelero_Z_Value | g;
            break;*/
        }
            
        EN_PIN_LVL_CHANGE();
        char str2[80];
        sprintf(str2, "X = %d | Y = %d | Z = %d\r\n", Accelero_X_Value, Accelero_Y_Value, Accelero_Z_Value);
        HAL_UART_Transmit(&huart2, (uint8_t*) str2, strlen(str2), HAL_MAX_DELAY);
           
        
        //deassert EN-pin, for power consumption and read new data
        
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