#include "main.h"
#include "stm32f0xx_hal.h"

void SystemClock_Config(void);

#define LED_PORT GPIOC
#define PIN_LED_RED GPIO_PIN_6
#define PIN_LED_BLUE GPIO_PIN_7
#define PIN_LED_ORANGE GPIO_PIN_8
#define PIN_LED_GREEN GPIO_PIN_9
#define ALL_LEDS (PIN_LED_RED | PIN_LED_BLUE | PIN_LED_ORANGE | PIN_LED_GREEN)

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
  g.Pin = GPIO_PIN_0; // PC0 is ADC channel 10
  g.Mode = GPIO_MODE_ANALOG; // Set to analog mode
  g.Pull = GPIO_NOPULL; // No pull-up or pull-down
  HAL_GPIO_Init(GPIOC, &g);
}

static void ADC1_Init(void) {
  __HAL_RCC_GPIOC_CLK_ENABLE(); // Enable clock for GPIOC
  RCC->APB2ENR |= RCC_APB2ENR_ADCEN; // Enable clock for ADC1

  RCC->CR2 |= RCC_CR2_HSI14ON; // Enable HSI14 oscillator
  while (!(RCC->CR2 & RCC_CR2_HSI14RDY)); // Wait until HSI14 is ready

  if(ADC1->CR & ADC_CR_ADEN) {
    ADC1->CR |= ADC_CR_ADDIS; // Disable ADC if it is enabled
    while (ADC1->CR & ADC_CR_ADEN); // Wait until ADC is disabled
  }

  ADC1->CR |= ADC_CR_ADCAL; // Start calibration
  while (ADC1->CR & ADC_CR_ADCAL); // Wait until calibration is complete

  ADC1->CFGR1 = 0; // Reset CFGR1
  ADC1->CFGR1 |= ADC_CFGR1_RES_1; // Set resolution to 8 bits
  
  ADC1->CHSELR |= ADC_CHSELR_CHSEL10; // Select ADC channel 10
  ADC1->SMPR = ADC_SMPR_SMP_2 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_0; // Set sampling time to 7.5 cycles

  ADC1->ISR = ADC_ISR_ADRDY | ADC_ISR_EOC | ADC_ISR_EOSEQ | ADC_ISR_OVR; // Clear ADRDY flag

  ADC1->CR |= ADC_CR_ADEN; // Enable ADC
  while (!(ADC1->ISR & ADC_ISR_ADRDY)); // Wait until ADC is
}

static uint8_t ADC1_Read(void) {
  ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOSEQ | ADC_ISR_OVR; // Clear EOC, EOSEQ, and OVR flags
  ADC1->CR |= ADC_CR_ADSTART; // Start conversion
  while (!(ADC1->ISR & ADC_ISR_EOC)); // Wait until conversion is complete
  return (uint8_t)(ADC1->DR & 0xFF); // Return the converted value
}

static void DAC_GPIO_Init(void) {
  __HAL_RCC_GPIOA_CLK_ENABLE(); // Enable clock for GPIOA

  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_4; // PA4 is DAC output
  g.Mode = GPIO_MODE_ANALOG; // Set to analog mode
  g.Pull = GPIO_NOPULL; // No pull-up or pull-down
  HAL_GPIO_Init(GPIOA, &g);
}

static void DAC1_Init(void) {
  RCC->APB1ENR |= RCC_APB1ENR_DACEN; // Enable clock for DAC
  DAC->CR &= ~DAC_CR_EN1; // Disable DAC channel 1
  DAC->CR &= ~DAC_CR_TSEL1; // Clear trigger selection
  DAC->CR |= (0x7U << DAC_CR_TSEL1_Pos); // Set trigger to software trigger
  DAC->CR |= DAC_CR_TEN1; // Enable DAC channel 1
  DAC->CR |= DAC_CR_EN1; // Enable DAC channel 1
}

static void DAC1_Write(uint8_t sample) {
  DAC->DHR8R1 = sample; // Write 8-bit value to DAC channel 1
  DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1; // Trigger the DAC conversion
}

#define THR_LED1 64u
#define THR_LED2 128u
#define THR_LED3 192u
#define THR_LED4 255u

static const uint8_t sine_table[32] = {
  127, 151, 175, 197, 216, 232, 244, 251,
  254, 251, 244, 232, 216, 197, 175, 151,
  127, 102, 78, 56, 37, 21, 9, 2, 
  0, 2, 9, 21, 37, 56, 78, 102
};

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
  LED_Init();
  ADC_GPIO_Init();
  ADC1_Init();
  DAC_GPIO_Init();
  DAC1_Init();

  uint32_t sine_idx = 0;

  while (1)
  {
    uint8_t adc_value = ADC1_Read();

    HAL_GPIO_WritePin(LED_PORT, ALL_LEDS, GPIO_PIN_RESET); // Turn off all LEDs
    if (adc_value >= THR_LED1) {
      HAL_GPIO_WritePin(LED_PORT, PIN_LED_RED, GPIO_PIN_SET);
    }
    if (adc_value >= THR_LED2) {
      HAL_GPIO_WritePin(LED_PORT, PIN_LED_BLUE, GPIO_PIN_SET);
    }
    if (adc_value >= THR_LED3) {
      HAL_GPIO_WritePin(LED_PORT, PIN_LED_GREEN, GPIO_PIN_SET);
    }
    if (adc_value >= THR_LED4) {
      HAL_GPIO_WritePin(LED_PORT, PIN_LED_ORANGE, GPIO_PIN_SET);
    }

    DAC1_Write(sine_table[sine_idx]); // Output sine wave value to DAC
    sine_idx = (sine_idx + 1) % 32; // Increment index and wrap around after 32 samples

    HAL_Delay(1); // Delay for a short period to control the update rate
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