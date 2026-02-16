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

  // Exercise 3.1: Using the Timer Peripheral to Generate Periodic Interrupts
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN; // Enable GPIOC clock
  GPIOC->MODER &= ~((3 << (8 * 2)) | (3 << (9 * 2))); //Clear bits PC8(orange), and PC9(green).
  GPIOC->MODER |= ((1 << (8 * 2)) | (1 << (9 * 2))); // Set bits to 01 (output mode)
  GPIOC->ODR |= (1 << 8); // Set PC8(orange) high (initial state)
  GPIOC->ODR &= ~(1 << 9); // Set PC9(green) low (initial state)

  // // Enable TIM2
  // RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // TIM2->PSC = 7999; // Set prescaler to 7999
  // TIM2->ARR = 250;  // Set auto-reload value to 250
  // TIM2->DIER |= TIM_DIER_UIE;

  // // Enable TIM2 interrupt
  // NVIC_EnableIRQ(TIM2_IRQn);
  // TIM2->CR1 |= TIM_CR1_CEN; // Start the timer

  // Exercise 3.2: Configure Timer Channels to PWM Mode
  // For this part, we will be using channel 1 and channel 2 of TIM2.
  // Target frequency = 800 Hz, clock frequency = 8 MHz.
  // Using the formula f = clock / (PSC + 1) / (ARR + 1), we can calculate the values for PSC and ARR.
  // PSC = 79, ARR = 125
  TIM3->PSC = 79; // Set prescaler to 79
  TIM3->ARR = 125; // Set auto-reload value to 125
  // CCR1 = 25, CCR2 = 25 for 20% duty cycle
  TIM3->CCR1 = 25; // Set capture/compare register 1 to 25
  TIM3->CCR2 = 25; // Set capture/compare register 2 to 25

  // Set channel 1 to PWM Mode 2, active low
  TIM3->CCMR1 &= ~TIM_CCMR1_OC1M; // Clear output compare mode bits for channel 1
  TIM3->CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos); // Set output compare mode to PWM Mode
  TIM3->CCER |= TIM_CCER_CC1E; // Enable output for channel 1

  // Set channel 2 to PWM Mode 1, active high
  TIM3->CCMR1 &= ~TIM_CCMR1_OC2M; // Clear output compare mode bits for channel 2
  TIM3->CCMR1 |= (5 << TIM_CCMR1_OC2M_Pos); // Set output compare mode to PWM Mode
  TIM3->CCER |= TIM_CCER_CC2E; // Enable output for channel 2

  // Enable preload for CCR1 and CCR2
  TIM3->CCMR1 |= TIM_CCMR1_OC1PE; // Enable preload for channel 1
  TIM3->CCMR1 |= TIM_CCMR1_OC2PE; // Enable preload for channel 2
  TIM3->CR1 |= TIM_CR1_ARPE; // Enable auto-reload preload

  // Enable outputs
  TIM3->CCER |= TIM_CCER_CC1E; // Enable output for channel 1
  TIM3->CCER |= TIM_CCER_CC2E; // Enable output for channel 2
  TIM3->CR1 |= TIM_CR1_CEN; // Start the timer

  // Exercise 3.3: configuring pin alternate functions
  

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

void TIM2_IRQHandler() {
  if(TIM2->SR & TIM_SR_UIF) {
    TIM2->SR &= ~TIM_SR_UIF; // Clear the update interrupt flag
    GPIOC->ODR ^= (1 << 8); // Toggle PC8(green)
    GPIOC->ODR ^= (1 << 9); // Toggle PC9(blue)
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
