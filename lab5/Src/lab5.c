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
