#include "main.h"
#include "stm32f0xx_hal.h"

#define ASSERT(cond) do { if (!(cond)) Error_Handler(); } while (0)

void SystemClock_Config(void);

volatile uint8_t currentState = 0;

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

  RCC->AHBENR |= RCC_AHBENR_GPIOCEN; // Enable GPIOC clock

  GPIOC->MODER &= ~((3 << (6 * 2)) | (3 << (7 * 2)) | (3 << (8 * 2)) | (3 << (9 * 2))); // Clear bits PC6(red), PC7(blue), PC8(orange), and PC9(green).
  GPIOC->MODER |= ((1 << (6 * 2)) | (1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2))); // Set bits to 01 (output mode)

  GPIOC->ODR |= (1 << 9); // Set PC9(green) high (initial state)
  GPIOC->ODR |= (1 << 6); // Set PC6(red) high (initial state)

  // Checkoff 2: Using the EXTI peripheral to generate interrupts with the user button.
  ASSERT((EXTI->IMR & (1 << 0)) == 0);
  HAL_GPIO_Configure_Rising_Edge_PA0();
  ASSERT((EXTI->IMR & (1 << 0)) != 0);

  SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0; // 0000 = GPIOA

  while (1)
  {
    GPIOC->ODR ^= (1 << 6); // Toggle PC6(red)
    HAL_Delay(400); // 1 second delay
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

void HAL_GPIO_Configure_Rising_Edge_PA0(void) {

  RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // Enable user button
  GPIOA->MODER &= ~(3 << (0 * 2)); // Set the user button to input
  GPIOA->OSPEEDR &= ~(3 << (0 * 2)); // Set the speed to low

  // PUPDR: 10 = pull-down, this is necessary to reduce noise in the system which can cause false triggers.
  // Basically, the pull down resistor pulls the state to ground when the button is not being pressed.
  GPIOA->PUPDR &= ~(3 << (0 * 2));
  GPIOA->PUPDR |=  (2 << (0 * 2));

  // EXTI_IMR, EXTI_FTSR, and EXTI_RTSR configuration
  // For these registers we use |= because we want the values
  // already in the register to stay the same.
  EXTI->IMR |= (1 << 0);
  EXTI->RTSR |= (1 << 0);
  // We want to make sure this bit is zero
  EXTI->FTSR &= ~(1 << 0);

  // 2.3 Setting the SYSCFG Pin Multiplexer
  // It looks like EXTI0 has [3:0] in SYSCFG_EXTICR1
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  ASSERT((RCC->APB2ENR & RCC_APB2ENR_SYSCFGEN) != 0);
  SYSCFG->EXTICR[0] &= ~(0xF << 0);
  ASSERT((SYSCFG->EXTICR[0] & 0xF) == 0);

  // 2.4 Enable and set priority of the EXTI Interrupt
  // 
  NVIC_EnableIRQ(EXTI0_1_IRQn);

  // For second checkoff, comment out 2 and 3, uncomment 1, and run it. Then Reverse it to show the different operations with different priorities.
  NVIC_SetPriority(EXTI0_1_IRQn, 1);
  // NVIC_SetPriority(SysTick_IRQn, 2);  // SysTick = medium
  // NVIC_SetPriority(EXTI0_1_IRQn, 3);  // EXTI = low
}

void EXTI0_1_IRQHandler() {
  if(EXTI->PR & (1 << 0)) {
    EXTI->PR = (1 << 0);
  }
  if(!currentState) {
    currentState = 1;
    GPIOC->ODR |= (1 << 8);
    GPIOC->ODR &= ~(1 << 9);
  }
  else {
    currentState = 0;
    GPIOC->ODR |= (1 << 9);
    GPIOC->ODR &= ~(1 << 8);
  }

  // Checkoff 2: Long running delay
  volatile uint32_t i;
  for(i = 0; i < 1500000; i++) {
    
  }

  // Toggle LEDs after delay
  if(currentState) {
    currentState = 0;
    GPIOC->ODR |= (1 << 9);
    GPIOC->ODR &= ~(1 << 8);
  }
  else {
    currentState = 1;
    GPIOC->ODR |= (1 << 8);
    GPIOC->ODR &= ~(1 << 9);
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
