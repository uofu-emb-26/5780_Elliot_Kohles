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
  TIM3_PWM_Init();
  LED_GPIO_Init();

  while (1)
  {
    for (int i = 0; i <= 9999; i += 100)
    {
    TIM3->CCR1 = i;
    TIM3->CCR2 = 9999 - i;
    for (volatile int d = 0; d < 5000; d++);
    }
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

void LED_GPIO_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // PC8 and PC9 alternate function
    GPIOC->MODER &= ~((3 << (6*2)) | (3 << (7*2)));
    GPIOC->MODER |=  ((2 << (6*2)) | (2 << (7*2)));

    // AF1 = TIM3
    // PC6 - TIM3 Channel 1
    // PC7 - TIM3 Channel 2
    GPIOC->AFR[1] &= ~((0xF << (6*4)) | (0xF << (7*4)));
    GPIOC->AFR[1] |=  ((1 << (6*4)) | (1 << (7*4)));
}

void TIM3_PWM_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;


    TIM3->PSC = 0;
    TIM3->ARR = 9999;

    TIM3->CR1 |= TIM_CR1_ARPE; // Auto-reload preload enable
    TIM3->EGR |= TIM_EGR_UG; // Generate an update event to load the prescaler value immediately

    // CCMR1: Capture/Compare Mode Register 1
    // CC1S[1:0] = 00 (output)
    // CC2S[9:8] = 00 (output)
    // OC1M[6:4] = 111 (PWM mode 2)
    // OC2M[14:12] = 110 (PWM mode 1)
    // OC1PE[3]
    // OC2PE[11]


    TIM3->CCMR1 &= ~(3 << 0); // Set CC1S to output
    TIM3->CCMR1 &= ~(3 << 8); // Set CC2S to output
    TIM3->CCMR1 |= (7 << 4); // 3.2 Part c: Set OC1M to PWM mode 2

    // 3.2 Part d: Set OC2M to PWM mode 1
    TIM3->CCMR1 &= ~(7 << 12); // Clear OC2M bits
    TIM3->CCMR1 |= (6 << 12); // Set OC2M to PWM mode 1
    // 3.2 Part e: Enable OC1PE and OC2PE
    TIM3->CCMR1 |= (1 << 3); // Enable preload for CCR1
    TIM3->CCMR1 |= (1 << 11); // Enable preload for CCR2

    TIM3->CCER |= (1 << 0) | (1 << 4); // Enable output for CC1 and CC2
    TIM3->CCR1 = 9900;
    TIM3->CCR2 = 9900;

    TIM3->CR1 |= TIM_CR1_CEN; // Enable the timer
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
