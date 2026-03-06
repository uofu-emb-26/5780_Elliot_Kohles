#include "main.h"
#include "stm32f0xx_hal.h"

void SystemClock_Config(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // Enable clock for GPIOB
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN; // Enable clock for GPIOC
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // Enable clock for I2C2

  //Set PB11 to alternate function mode, open-drain output type, and select I2C2_SDA as its
  //alternate function.
  GPIOB->MODER &= ~(3 << (11 * 2)); // Clear mode bits for PB11
  GPIOB->MODER |= (2 << (11 * 2)); // Set PB11 to alternate function mode

  GPIOB->OTYPER |= (1 << 11); // Set PB11 to open-drain output type
  
  GPIOB->AFR[1] &= ~(0xF << (11 - 8) *4); // Clear I2C2_SDA alternate function bits for PB11
  GPIOB->AFR[1] |= (1 << ((11 - 8) * 4)); // Select I2C2_SDA as alternate function for PB11

  // Set PB13 to alternate function mode, open-drain output type, and select I2C2_SCL as its
  // alternate function.
  GPIOB->MODER &= ~(3 << (13 * 2)); // Clear mode bits for PB13
  GPIOB->MODER |= (2 << (13 * 2)); // Set PB13 to alternate function mode

  GPIOB->OTYPER |= (1 << 13); // Set PB13 to open-drain output type
  GPIOB->AFR[1] &= ~(0xF << (13 - 8) *4); // Clear I2C2_SCL alternate function bits for PB13
  GPIOB->AFR[1] |= (1 << ((13 - 8) * 4)); // Select I2C2_SCL as alternate function for PB13

  //Set PB14 to output mode, push-pull output type, and initialize/set the pin high.
  GPIOB->MODER &= ~(3 << (14 * 2)); // Set PB14 to output mode
  GPIOB->MODER |= (1 << (14 * 2)); // Set PB14 to output mode
  GPIOB->OTYPER &= ~(1 << 14); // Set PB14 to push-pull output type
  GPIOB->ODR |= (1 << 14); // Initialize/set PB14 high

  // Set PC0 to output mode, push-pull output type, and initialize/set the pin high.
  GPIOC->MODER &= ~(3 << (0 * 2)); // Set PC0 to output mode
  
  GPIOC->MODER |= (1 << (0 * 2)); // Set PC0 to output mode
  GPIOC->OTYPER &= ~(1 << 0); // Set PC0 to push-pull output type
  GPIOC->ODR |= (1 << 0); // Initialize/set PC0 high

  // Establish I2C2 communication
  I2C2->CR1 &= ~I2C_CR1_PE; // Disable I2C2 before configuration
  I2C2->TIMINGR = 0x2000090E; // Configure timing for 100 kHz I2C communication
  I2C2->CR1 |= I2C_CR1_PE; // Enable I

  // Read WHO_AM_I register 0xD3 with gyroscope address 0x69
  // Write register address 
  // SADD = 0x69
  I2C2->CR2 &= ~((0x7F << 16) | (0x3FF << 0)); // Clear SADD and NBYTES fields
  
  // Set parameters for the transfer: SADD = 0x69, NBYTES = 1, and generate START condition
  I2C2->CR2 |= (1 << 16) 
  IC2C->CR2 |= (0x69 << 1); // Set SADD to 0x69, NBYTES to 1, and generate START condition
  IC2C->CR2 &= ~(1 << 10); // RD_WRN = 0 for write operation

  // Write 
  I2C2->CR2 &= ~I2C_CR2_RD_WRN;
  I2C2->CR2 |= I2C_CR2_START; // Generate START condition
  while(!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)));

  if (I2C2->ISR & (I2C_ISR_NACKF)) {
    // Handle NACK error
    I2C2->ICR |= I2C_ICR_NACKCF; // Clear NACK flag
  } else {
    I2C2->TXDR = 0xD3; // Write register address to TXDR
    while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF))); // Wait for transfer complete or NACK

    if (I2C2->ISR & (I2C_ISR_NACKF)) {
      // Handle NACK error
      I2C2->ICR |= I2C_ICR_NACKCF; // Clear NACK flag
    } else {
      // Read data from WHO_AM_I register
      I2C2->CR2 |= I2C_CR2_RD_WRN; // Set read mode
      I2C2->CR2 |= I2C_CR2_START; // Generate repeated START condition
      while(!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF))); // Wait for data reception or NACK

      if (I2C2->ISR & (I2C_ISR_NACKF)) {
        // Handle NACK error
        I2C2->ICR |= I2C_ICR_NACKCF; // Clear NACK flag
      } else {
        uint8_t who_am_i = I2C2->RXDR; // Read received data from RXDR
        // who_am_i should be 0xD3 for the L3GD20H gyroscope
      }
    }
    
  }

  // Send register address
  I2C2->TXDR = 0x0F;
  while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF))); // Wait for transfer complete or NACK
  
  // Restart and Read
  I2C2->CR2 &= ~((0x7F << 16) | (0x3FF)); // Clear SADD and NBYTES fields
  I2C2->CR2 |= (1 << 16) | (0x69 << 1) | I2C_CR2_RD_WRN; // Set SADD to 0x69, NBYTES to 1, set read mode, and generate START condition

  I2C2->CR2 |= I2C_CR2_RD_WRN; // Set read mode
  I2C2->CR2 |= I2C_CR2_START; // Generate repeated START condition

  // Wait for RXNE or NACK
  while(!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF))); // Wait for data reception or NACK

  // Read value
  uint8_t who = I2C2->RXDR; // Read received data from RXDR

  // Wait for TC
  while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF))); // Wait for transfer complete or NACK

  // Generate STOP condition
  I2C2->CR2 |= I2C_CR2_STOP;

  // Check value of WHO_AM_I register
  if (who == 0xD3) {
    // WHO_AM_I register value is correct, indicating successful communication with the gyroscope
    // Add code here to indicate success, toggling an LED

  }

  else {
    // WHO_AM_I register value is incorrect, indicating a communication error with the gyroscope
    // You can add code here to indicate an error, such as toggling an LED or sending an error message over USART3
  }
  while (1)
  {
    
  }
  return -1;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add their own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
  /* User can add their own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
