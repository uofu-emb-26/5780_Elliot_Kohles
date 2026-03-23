#include "main.h"
#include "stm32f0xx_hal.h"

void SystemClock_Config(void);

#define LED_PORT GPIOC
#define LED_PIN_RED GPIO_PIN_6
#define LED_PIN_BLUE GPIO_PIN_7
#define LED_PIN_ORANGE GPIO_PIN_8
#define LED_PIN_GREEN GPIO_PIN_9
#define ALL_LEDS (LED_PIN_RED | LED_PIN_BLUE | LED_PIN_ORANGE | LED_PIN_GREEN)

static void LED_Init(void) {
  __HAL_RCC_GPIOC_CLK_ENABLE(); // Enable clock for GPIOC

  GPIO_InitTypeDef g = {0};
  g.Pin = ALL_LEDS;
  g.Mode = GPIO_MODE_OUTPUT_PP; // Push-pull output
  g.Pull = GPIO_NOPULL; // No pull-up or pull-down
  g.Speed = GPIO_SPEED_FREQ_LOW; // Low speed
  HAL_GPIO_Init(LED_PORT, &g);

  HAL_GPIO_WritePin(LED_PORT, ALL_LEDS, GPIO_PIN_RESET); // Start with all LEDs off
}

static void ADC_GPIO_Init(void) {
  __HAL_RCC_GPIOC_CLK_ENABLE(); // Enable clock for GPIOC

  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_0; // PC0 for ADC input
  g.Mode = GPIO_MODE_ANALOG; // Analog mode
  g.Pull = GPIO_NOPULL; // No pull-up or pull-down
  HAL_GPIO_Init(GPIOC, &g);
}

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
