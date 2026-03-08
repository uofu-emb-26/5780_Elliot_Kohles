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

  // Initialize GPIO and I2C hardware
  INIT_STMHARDWARE();

  // Establish I2C2 communication
  ESTABLISH_I2C2_COMMS();

  // Read WHO_AM_I register from gyroscope
  //I2C_READ_REGISTER();

  while (1)
  {
    // Temporary LED toggle verification to confirm
    // WHO_AM_I is being read correctly.

    GPIOC->ODR ^= (1 << 7); // Toggle blue LED on PC7

    uint8_t who = I2C_READ_REGISTER(0x69, 0x0F); // Read WHO_AM_I register (0x0F) from gyroscope at address 0x69

    if(who == 0xD3) {
      GPIOC->ODR |= (1 << 9); // Toggle green LED on PC9
    }
    else {
      GPIOC->ODR |= (1 << 6); // Toggle red LED on PC6
    }
    HAL_Delay(500); // Delay for 500 milliseconds
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

void INIT_STMHARDWARE(void) {

  // Setup test LED pins
  // Enable GPIOC clock
  // RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  // Set PC6, PC8 and PC9 to output mode
  GPIOC->MODER &= ~(3 << (6 * 2));
  GPIOC->MODER &= ~(3 << (7 * 2));
  GPIOC->MODER &= ~(3 << (9 * 2));

  GPIOC->MODER |= (1 << (6 * 2));
  GPIOC->MODER |= (1 << (7 * 2));
  GPIOC->MODER |= (1 << (9 * 2));


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
}

void ESTABLISH_I2C2_COMMS(void) {
  I2C2->CR1 &= ~I2C_CR1_PE; // Disable I2C2 before configuration
  I2C2->TIMINGR = 0x2000090E; // Configure timing for 100 kHz I2C communication
  I2C2->CR1 |= I2C_CR1_PE; // Enable I2C2

  // Configure write transfer to gyroscope at address 0x69
  I2C2->CR2 &= ~((0x7F << 16) | (0x3FF << 0)); // Clear SADD and NBYTES fields

  I2C2->CR2 |= (1 << 16); // Set NBYTES to 1
  I2C2->CR2 |= (0x69 << 1); // Set SADD to 0x69
  I2C2->CR2 &= ~I2C_CR2_RD_WRN; // RD_WRN = 0 for write operation
  I2C2->CR2 |= I2C_CR2_START; // Generate START condition
}

// Function to read a single byte from a specified register of the slave device. This function
// should assist in reading the WHO_AM_I register of the gyroscope.
uint8_t I2C_READ_REGISTER(uint8_t slave_addr, uint8_t reg) {
  uint8_t data;

  // Write register address to slave
  I2C2->CR2 &= ~((0x7F << 16) | (0x3FF << 0)); // Clear SADD and NBYTES fields

  I2C2->CR2 |= (1 << 16) | (slave_addr << 1); // Set NBYTES to 1 and SADD to slave address
  I2C2->CR2 &= ~I2C_CR2_RD_WRN; // Set write mode
  I2C2->CR2 |= I2C_CR2_START; // Generate START condition

  // Wait for TXIS or NACK
  while(!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)));
  I2C2->TXDR = reg; // Write register address to TXDR

  // Wait for transfer complete or NACK
  while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)));

  // Restart read transfer
  I2C2->CR2 &= ~((0x7F << 16) | (0x3FF << 0)); // Clear SADD and NBYTES fields

  I2C2->CR2 |= (1 << 16) | (slave_addr << 1) | I2C_CR2_RD_WRN; // Set NBYTES to 1, SADD to slave address, and set read mode
  I2C2->CR2 |= I2C_CR2_START; // Generate repeated START condition

  // Wait for RXNE or NACK
  while(!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF)));
  data = I2C2->RXDR; // Read received data from RXDR

  // Wait for transfer complete or NACK
  while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)));

  // Generate STOP condition
  I2C2->CR2 |= I2C_CR2_STOP;

  return data;
}

// Function to read multiple bytes from consecutive registers of the slave device. This
// function should assist in reading the gyroscope’s output data registers.
void I2C_READ_REGISTERS(uint8_t slave_addr, uint8_t reg, uint8_t *buffer, uint16_t length) {
  int i;
  
  // Write register address to slave
  I2C2->CR2 &= ~((0x7F << 16) | (0x3FF << 0)); // Clear SADD and NBYTES fields

  I2C2->CR2 |= (1 << 16) | (slave_addr << 1); // Set NBYTES to 1 and SADD to slave address
  I2C2->CR2 &= ~I2C_CR2_RD_WRN; // Set write mode
  I2C2->CR2 |= I2C_CR2_START; // Generate START condition

  // Wait for TXIS or NACK
  while(!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)));
  I2C2->TXDR = reg; // Write register address to TXDR

  // Wait for transfer complete or NACK
  while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)));

  // Restart read transfer
  I2C2->CR2 &= ~((0x7F << 16) | (0x3FF << 0)); // Clear SADD and NBYTES fields

  I2C2->CR2 |= (length << 16) | (slave_addr << 1) | I2C_CR2_RD_WRN; // Set NBYTES to length, SADD to slave address, and set read mode
  I2C2->CR2 |= I2C_CR2_START; // Generate repeated START condition

  for (i = 0; i < length; i++) {
    // Wait for RXNE or NACK
    while(!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF)));
    buffer[i] = I2C2->RXDR; // Read received data from RXDR
  }

  // Wait for transfer complete or NACK
  while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)));

  // Generate STOP condition
  I2C2->CR2 |= I2C_CR2_STOP;
}

// void READ_WHOAMI(void) {
//   // Wait for TXIS or NACK
//   while(!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)));

//   // Send register address 0x0F (WHO_AM_I register)
//   I2C2->TXDR = 0x0F; // Write WHO_AM_I register address to TXDR

//   // Wait for transfer complete or NACK
//   while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)));

//   // Restart read transfer
//   I2C2->CR2 &= ~((0x7F << 16) | (0x3FF << 0)); // Clear SADD and NBYTES fields
//   I2C2->CR2 |= (1 << 16) | (0x69 << 1) | I2C_CR2_RD_WRN; // Set NBYTES to 1, SADD to 0x69, set read mode, and generate START condition
//   I2C2->CR2 |= I2C_CR2_START; // Generate repeated START condition

//   // Wait for RXNE or NACK
//   while(!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF)));

//   // Read value from WHO_AM_I register
//   uint8_t who_am_i = I2C2->RXDR; // Read received data from RXDR

//   // Wait for transfer complete or NACK
//   while(!(I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)));

//   // Generate STOP condition
//   I2C2->CR2 |= I2C_CR2_STOP;

//   // Verify WHO_AM_I value and set LEDs accordingly
//   VERIFY_WHOAMI(who_am_i);
// }

// void VERIFY_WHOAMI(uint8_t who_am_i) {
//   // enable and turn on the red LED if the value read from the WHO_AM_I register is incorrect (not 0xD3)
//   // otherwise, enable and turn on the green LED if the value read from the WHO_AM_I register is correct (0xD3)
//   if (who_am_i == 0xD3) {
//     GPIOC->ODR |= (1 << 8);
//   } else {
//     GPIOC->ODR |= (1 << 9);
//   }
// }

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
