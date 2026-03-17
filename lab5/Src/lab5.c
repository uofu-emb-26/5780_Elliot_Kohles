#include "main.h"
#include "stm32f0xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

void SystemClock_Config(void);

// Device definitions
#define GYRO_ADDR_7BIT 0x69 // Gyroscope I2C address
#define WHO_AM_I_REG 0x0F // WHO_AM_I register address
#define CTRL_REG1 0x20 // Control register 1 address
#define OUT_X_L 0x28 // Output register for X-axis low byte
#define AUTO_INCREMENT 0x80 // Auto-increment bit for multi-byte reads
#define WHO_AM_I_EXPECTED 0xD3 // Expected value in WHO_AM_I register

// LED and gyro definitions
#define LED_UP_PIN 6 // Red LED on PC6
#define LED_DOWN_PIN 7 // Blue LED on PC7
#define LED_LEFT_PIN 8 // Green LED on PC8
#define LED_RIGHT_PIN 9 // Green LED on PC9

#define GYRO_MODE_PIN 0 // Gyroscope mode control pin on PC0
#define GYRO_POWER_PIN 14 // Gyroscope power control pin on PB14

#define AXIS_THRESHOLD 2500 // Threshold to ignore small movements.

// Timing definitions
static volatile uint32_t g_ms_ticks = 0; // Millisecond tick counter

// void SysTick_Handler(void) {
//   g_ms_ticks++;
// }

void delay_init(void) {
  SystemCoreClockUpdate(); // Update SystemCoreClock variable
  // Configure SysTick to generate an interrupt every 1 ms
  SysTick_Config(SystemCoreClock / 1000);
}

void delay_ms(uint32_t ms) {
  uint32_t start_tick = g_ms_ticks;
  while ((g_ms_ticks - start_tick) < ms) {
    // Wait until the specified number of milliseconds has elapsed
  }
}

// GPIO and I2C initialization functions
static inline void set_pin(GPIO_TypeDef *port, uint16_t pin) {
  port->BSRR = (1 << pin);
}

static inline void clear_pin(GPIO_TypeDef *port, uint16_t pin) {
  port->BSRR = (1 << (pin + 16));
}

void clear_leds(void) {
  clear_pin(GPIOC, LED_UP_PIN);
  clear_pin(GPIOC, LED_DOWN_PIN);
  clear_pin(GPIOC, LED_LEFT_PIN);
  clear_pin(GPIOC, LED_RIGHT_PIN);
}

void GPIO_init(void) {
  // Enable GPIOC and GPIOB clocks
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOBEN;

  // Configure LED pins as output
  for (int pin = LED_UP_PIN; pin <= LED_RIGHT_PIN; pin++) {
    GPIOC->MODER &= ~(3 << (pin * 2)); // Clear mode bits
    GPIOC->MODER |= (1 << (pin * 2)); // Set to output mode
    GPIOC->OTYPER &= ~(1 << pin); // Set to push-pull
    GPIOC->PUPDR &= ~(3 << (pin * 2)); // No pull-up/pull-down
    clear_pin(GPIOC, pin); // Ensure LEDs are off initially
  }

  // Configure gyroscope control pins
  // PC0
  GPIOC->MODER &= ~(3 << (GYRO_MODE_PIN * 2)); // Clear mode bits for mode pin
  GPIOC->MODER |= (1 << (GYRO_MODE_PIN * 2)); // Set mode pin to output
  GPIOC->OTYPER &= ~(1 << GYRO_MODE_PIN); // Set mode pin to push-pull
  GPIOC->PUPDR &= ~(3 << (GYRO_MODE_PIN * 2));
  set_pin(GPIOC, GYRO_MODE_PIN);

  // PB14
  GPIOB->MODER &= ~(3 << (GYRO_POWER_PIN * 2)); // Clear mode bits for power pin
  GPIOB->MODER |= (1 << (GYRO_POWER_PIN * 2)); // Set power pin to output
  GPIOB->OTYPER &= ~(1 << GYRO_POWER_PIN); // Set power pin to push-pull
  GPIOB->PUPDR &= ~(3 << (GYRO_POWER_PIN * 2));
  set_pin(GPIOB, GYRO_POWER_PIN);

  // Set PB11 SDA, PB13 SCL as alternate function open-drain AF1
  const uint8_t sda_pin = 11;
  const uint8_t scl_pin = 13;

  GPIOB->MODER &= ~((3 << (sda_pin * 2)) | (3 << (scl_pin * 2))); // Clear mode bits
  GPIOB->MODER |= (2 << (sda_pin * 2)) | (2 << (scl_pin * 2)); // Set to alternate function mode
  GPIOB->OTYPER |= (1 << sda_pin) | (1 << scl_pin); // Set to open-drain
  GPIOB->PUPDR &= ~((3 << (sda_pin * 2)) | (3 << (scl_pin * 2))); // No pull-up/pull-down
  GPIOB->AFR[1] &= ~((0xF << ((sda_pin - 8) * 4)) | (0xF << ((scl_pin - 8) * 4))); // Clear alternate function bits
  GPIOB->AFR[1] |= (1 << ((sda_pin - 8) * 4)) | (1 << ((scl_pin - 8) * 4)); // Set alternate function to AF1 (I2C2)
}

// I2C helper functions

void I2C_init(void) {
  // Enable I2C2 clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

  I2C2->CR1 &= ~I2C_CR1_PE; // Disable I2C2 before configuration

  // Configure I2C timing for 100 kHz
  I2C2->TIMINGR = 0x2000090E; // This value may need to be adjusted based on the actual clock frequency

  // Clear state flags
  I2C2->ICR = I2C_ICR_TIMOUTCF | I2C_ICR_NACKCF | I2C_ICR_STOPCF | I2C_ICR_BERRCF | I2C_ICR_ARLOCF | I2C_ICR_OVRCF;

  // Enable I2C2
  I2C2->CR1 |= I2C_CR1_PE;
}

static inline void I2C2_start(uint8_t addr7, uint8_t nbytes, bool read) {
  uint32_t cr2 = I2C2->CR2;

  cr2 &= ~(I2C_CR2_SADD_Msk |
             I2C_CR2_NBYTES_Msk |
             I2C_CR2_RD_WRN |
             I2C_CR2_AUTOEND |
             I2C_CR2_RELOAD |
             I2C_CR2_START |
             I2C_CR2_STOP);

    cr2 |= ((uint32_t)addr7 << 1) & I2C_CR2_SADD_Msk;
    cr2 |= ((uint32_t)nbytes << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES_Msk;
    if (read) {
        cr2 |= I2C_CR2_RD_WRN;
    }
    cr2 |= I2C_CR2_START;

    I2C2->CR2 = cr2;
}

int I2C2_txis_nack_wait(void) {
  uint32_t timeout = 2000000;
    while ((I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) == 0) {
        if (timeout-- == 0) return -1;
    }
    if (I2C2->ISR & I2C_ISR_NACKF) {
        I2C2->ICR = I2C_ICR_NACKCF;
        return -2;
    }
    return 0;
}

int I2C2_nack_rxne_wait(void) {
  uint32_t timeout = 2000000;
    while ((I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF)) == 0) {
        if (timeout-- == 0) return -1;
    }
    if (I2C2->ISR & I2C_ISR_NACKF) {
        I2C2->ICR = I2C_ICR_NACKCF;
        return -2;
    }
    return 0;
}

int I2C2_tc_nack_wait(void) {
  uint32_t timeout = 2000000;
    while ((I2C2->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)) == 0) {
        if (timeout-- == 0) return -1;
    }
    if (I2C2->ISR & I2C_ISR_NACKF) {
        I2C2->ICR = I2C_ICR_NACKCF;
        return -2;
    }
    return 0;
}

void stop_and_clear(void) {
  I2C2->CR2 |= I2C_CR2_STOP; // Generate STOP condition
  while ((I2C2->ISR & I2C_ISR_STOPF) == 0); // Wait for STOP condition to be sent
  I2C2->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF; // Clear STOP and NACK flags
}

int gyro_write_reg(uint8_t reg, uint8_t value) {
  I2C2_start(GYRO_ADDR_7BIT, 2, false); // Start write transfer
  if (I2C2_txis_nack_wait() != 0) {
    //stop_and_clear();
    return -1; // Error during address transmission
  }
  I2C2->TXDR = reg; // Send register address
  if (I2C2_txis_nack_wait() != 0) {
    //stop_and_clear();
    return -2; // Error during register address transmission
  }
  I2C2->TXDR = value; // Send register value

  if (I2C2_tc_nack_wait() != 0) {
    //stop_and_clear();
    return -3; // Error during data transmission
  }
  stop_and_clear(); // Generate STOP condition
  return 0; // Success
}

int gyro_read_reg(uint8_t reg, uint8_t *value) {
  I2C2_start(GYRO_ADDR_7BIT, 1, false); // Start write transfer to send register address

  if (I2C2_txis_nack_wait() != 0) {
    //stop_and_clear();
    return -1; // Error during address transmission
  }
  I2C2->TXDR = reg; // Send register address
  
  if (I2C2_tc_nack_wait() != 0) {
    //stop_and_clear();
    return -2; // Error during register address transmission
  }

  I2C2_start(GYRO_ADDR_7BIT, 1, true); // Start read transfer
  
  if (I2C2_nack_rxne_wait() != 0) {
    //stop_and_clear();
    return -3; // Error during repeated start or address transmission
  }
  *value = (uint8_t)I2C2->RXDR; // Read register value

  if (I2C2_tc_nack_wait() != 0) {
    //stop_and_clear();
    return -4; // Error during data reception
  }
  stop_and_clear(); // Generate STOP condition
  return 0; // Success
}

int gyro_read_bytes(uint8_t start_reg, uint8_t *buf, uint8_t len) {
  I2C2_start(GYRO_ADDR_7BIT, 1, false); // Start write transfer to send register address

  if (I2C2_txis_nack_wait() != 0) {
    //stop_and_clear();
    return -1; // Error during address transmission
  }
  I2C2->TXDR = (uint8_t)(start_reg | AUTO_INCREMENT); // Send starting register address with auto-increment
  
  if (I2C2_tc_nack_wait() != 0) {
    //stop_and_clear();
    return -2; // Error during register address transmission
  }

  I2C2_start(GYRO_ADDR_7BIT, len, true); // Start read transfer for multiple bytes
  
  for (uint8_t i = 0; i < len; i++) {
    if (I2C2_nack_rxne_wait() != 0) {
      //stop_and_clear();
      return -3; // Error during repeated start or address transmission
    }
    buf[i] = (uint8_t)I2C2->RXDR; // Read byte into buffer
  }

  if (I2C2_tc_nack_wait() != 0) {
    //stop_and_clear();
    return -4; // Error during data reception
  }
  stop_and_clear(); // Generate STOP condition
  return 0; // Success
}

// Gyroscope helper functions
int gyro_init(void) {
  // Set CTRL_REG1 to normal mode with all axes enabled and ODR of 95 Hz
  return gyro_write_reg(CTRL_REG1, 0x0F); // Normal mode, all axes enabled, ODR = 95 Hz
}

int gyro_read_whoami(uint8_t *whoami) {
  return gyro_read_reg(WHO_AM_I_REG, whoami);
}

int gyro_read_axes(int16_t *x, int16_t *y) {
  uint8_t buf[4];
  int status = gyro_read_bytes(OUT_X_L, buf, 4); // Read 4 bytes starting from OUT_X_L (X_L, X_H, Y_L, Y_H)
  if (status != 0) {
    return status; // Return error code if read failed
  }
  *x = (int16_t)((uint16_t)buf[1] << 8 | buf[0]); // Combine low and high bytes for X-axis
  *y = (int16_t)((uint16_t)buf[3] << 8 | buf[2]); // Combine low and high bytes for Y-axis
  return 0; // Success
}

// LED control
void show_direction(int16_t x, int16_t y) {
  int32_t ax = (x < 0) ? -x : x; // Absolute value of X-axis
  int32_t ay = (y < 0) ? -y : y; // Absolute value of Y-axis

  // Clear all LEDs first
  clear_leds();

  if (ax < AXIS_THRESHOLD && ay < AXIS_THRESHOLD) {
    // No significant movement, keep all LEDs off
    return;
  }

  if (ay > ax) {
    // Y-axis dominant
    if (y > 0) {
      set_pin(GPIOC, LED_UP_PIN); // Positive Y movement
    } else {
      set_pin(GPIOC, LED_DOWN_PIN); // Negative Y movement
    }
  } else {
    // X-axis dominant
    if (x > 0) {
      set_pin(GPIOC, LED_RIGHT_PIN); // Positive X movement
    } else {
      set_pin(GPIOC, LED_LEFT_PIN); // Negative X movement
    }
  }
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Configure the system clock */
  SystemClock_Config();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  delay_init();
  GPIO_init();
  I2C_init();
  
  // Read and verify WHO_AM_I register
  uint8_t whoami = 0;
  (void)gyro_read_whoami(&whoami);

  // Initialize gyroscope settings
  gyro_init();
  HAL_Delay(300); // Delay to allow gyroscope to stabilize after initialization

  while (1)
  {
    int16_t x = 0, y = 0;
    if (gyro_read_axes(&x, &y) == 0) {
      show_direction(x, y); // Update LEDs based on gyroscope readings
    } else {
      // Handle read error
      // leds all off
      clear_leds();

      set_pin(GPIOC, LED_UP_PIN); // Turn on up LED to indicate error
      set_pin(GPIOC, LED_DOWN_PIN); // Turn on down LED to indicate error
    }
    HAL_Delay(100); // Delay between readings
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
