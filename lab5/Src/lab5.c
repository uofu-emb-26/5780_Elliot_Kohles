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
#define OUT_X_H 0x29 // Output register for X-axis high byte
#define OUT_Y_L 0x2A // Output register for Y-axis low byte
#define OUT_Y_H 0x2B // Output register for Y-axis high byte
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
  GPIOB->PUPDR &= ~((3 << (sda_pin * 2)) | (3 << (scl_pin * 2))); // Clear pull-up/pull-down
  GPIOB->PUPDR |= (1 << (sda_pin * 2)) | (1 << (scl_pin * 2)); // Enable pull-up resistors

  GPIOB->AFR[1] &= ~((0xF << ((sda_pin - 8) * 4)) | (0xF << ((scl_pin - 8) * 4))); // Clear alternate function bits
  GPIOB->AFR[1] |= (0x1 << ((11 - 8) * 4));
  GPIOB->AFR[1] |= (0x5 << ((13 - 8) * 4));
}

// I2C helper functions

void I2C_init(void) {
  // Enable I2C2 clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

  I2C2->CR1 &= ~I2C_CR1_PE; // Disable I2C2 before configuration

  // Reset I2C2
  RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
  RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;

  // Configure I2C timing for 100 kHz
  I2C2->TIMINGR = (1u << 28) | (4u << 20) | (2u << 16) | (0x0Fu << 8) | (0x13u << 0);

  // Clear state flags
  I2C2->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF | I2C_ICR_BERRCF | I2C_ICR_ARLOCF | I2C_ICR_OVRCF;

  // Enable I2C2
  I2C2->CR1 |= I2C_CR1_PE;
}

void clear_flags(void) {
  if(I2C2->ISR & I2C_ISR_NACKF) {
    I2C2->ICR = I2C_ICR_NACKCF;
  }
  if(I2C2->ISR & I2C_ISR_STOPF) {
    I2C2->ICR = I2C_ICR_STOPCF;
  }
  if(I2C2->ISR & I2C_ISR_BERR) {
    I2C2->ICR = I2C_ICR_BERRCF;
  }
  if(I2C2->ISR & I2C_ISR_ARLO) {
    I2C2->ICR = I2C_ICR_ARLOCF;
  }
  if(I2C2->ISR & I2C_ISR_OVR) {
    I2C2->ICR = I2C_ICR_OVRCF;
  }
}

static void gyro_write_reg(uint8_t reg, uint8_t value) {
  while(I2C2->ISR & I2C_ISR_BUSY); // Wait until I2C is not busy
  clear_flags(); // Clear any existing flags

  // Start I2C transfer
  I2C2->CR2 = ((uint32_t)(GYRO_ADDR_7BIT << 1) << I2C_CR2_SADD_Pos) |
  (2u << I2C_CR2_NBYTES_Pos) | I2C_CR2_AUTOEND; // Set slave address, number of bytes, and enable auto-end mode
  
  I2C2->CR2 |= I2C_CR2_START; // Generate START
  
  while(!(I2C2->ISR & I2C_ISR_TXIS)); // Wait for TXIS flag to be set
  I2C2->TXDR = reg; // Send register address

  while(!(I2C2->ISR & I2C_ISR_TXIS)); // Wait for TXIS flag to be set
  I2C2->TXDR = value; // Send register value

  while(!(I2C2->ISR & I2C_ISR_STOPF));
  I2C2->ICR = I2C_ICR_STOPCF; // Clear STOP flag
}

static uint8_t gyro_read_reg(uint8_t reg) {
  uint8_t value = 0;

  while(I2C2->ISR & I2C_ISR_BUSY); // Wait until I2C is not busy
  clear_flags(); // Clear any existing flags
  
  I2C2->CR2 = ((uint32_t)(GYRO_ADDR_7BIT << 1) << I2C_CR2_SADD_Pos) |
  (1u << I2C_CR2_NBYTES_Pos); // Set slave address and number of bytes.

  I2C2->CR2 |= I2C_CR2_START;
  
  while(!(I2C2->ISR & I2C_ISR_TXIS)); // Wait for TXIS flag to be set
  I2C2->TXDR = reg; // Send register address

  while(!(I2C2->ISR & I2C_ISR_TC)); // Wait for transfer complete

  // Read
  I2C2->CR2 = ((uint32_t)(GYRO_ADDR_7BIT << 1) << I2C_CR2_SADD_Pos) |
  (1u << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN | I2C_CR2_AUTOEND; // Set slave address, number of bytes, and enable auto-end mode

  I2C2->CR2 |= I2C_CR2_START; // Generate START

  while(!(I2C2->ISR & I2C_ISR_RXNE)); // Wait for RXNE flag to be set
  value = (uint8_t)I2C2->RXDR; // Read byte

  while(!(I2C2->ISR & I2C_ISR_STOPF));
  I2C2->ICR = I2C_ICR_STOPCF; // Clear

  return value;
}

static void gyro_read_bytes(uint8_t start_reg, uint8_t *buf, uint8_t len) {
  if(len == 0) return;
  
  while(I2C2->ISR & I2C_ISR_BUSY); // Wait until I2C is not busy
  clear_flags(); // Clear any existing flags
  
  uint8_t reg = start_reg | AUTO_INCREMENT; // Set auto-increment bit for multi-byte read

  // Send register
  I2C2->CR2 = ((uint32_t)(GYRO_ADDR_7BIT << 1) << I2C_CR2_SADD_Pos) |
  (1u << I2C_CR2_NBYTES_Pos); // Set slave address and number of bytes.

  I2C2->CR2 |= I2C_CR2_START; // Generate START

  while(!(I2C2->ISR & I2C_ISR_TXIS)); // Wait for TXIS flag to be set
  I2C2->TXDR = reg; // Send register address

  while(!(I2C2->ISR & I2C_ISR_TC)); // Wait for transfer complete

  // Read multiple bytes
  I2C2->CR2 = ((uint32_t)(GYRO_ADDR_7BIT << 1) << I2C_CR2_SADD_Pos) |
  ((uint32_t)len << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN | I2C_CR2_AUTOEND; // Set slave address, number of bytes, and enable auto-end mode

  I2C2->CR2 |= I2C_CR2_START; // Generate START

  for(uint8_t i = 0; i < len; i++) {
    while(!(I2C2->ISR & I2C_ISR_RXNE)); // Wait for RXNE flag to be set
    buf[i] = (uint8_t)I2C2->RXDR; // Read byte
  }

  while(!(I2C2->ISR & I2C_ISR_STOPF)); // Wait for STOP condition to be sent
  I2C2->ICR = I2C_ICR_STOPCF; // Clear STOP flag
}

// Gyroscope helper functions
void gyro_init(void) {
  // CTRL_REG1: Normal mode, all axes enabled, 100 Hz data rate
  gyro_write_reg(CTRL_REG1, 0x0F);
  HAL_Delay(10);
}

uint8_t gyro_read_whoami(void) {
  return gyro_read_reg(WHO_AM_I_REG);
}

int gyro_read_axes(int16_t *x, int16_t *y) {
  uint8_t buf[4];
  gyro_read_bytes(OUT_X_L, buf, 4); // Read 4 bytes starting from OUT_X_L (X_L, X_H, Y_L, Y_H)

  *x = (int16_t)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8)); // Combine low and high bytes for X-axis
  *y = (int16_t)((uint16_t)buf[2] | ((uint16_t)buf[3] << 8)); // Combine low and high bytes for Y-axis
  return 0;
}

// LED control
void show_direction(int16_t x, int16_t y) {
  int32_t ax = (x < 0) ? -(int32_t)x : (int32_t)x; // Absolute value of X-axis
  int32_t ay = (y < 0) ? -(int32_t)y : (int32_t)y; // Absolute value of Y-axis

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
  uint8_t whoami;
  whoami = gyro_read_whoami();

  // Indicate success or failure of WHO_AM_I check using LEDs
  if (whoami == WHO_AM_I_EXPECTED) {
    set_pin(GPIOC, LED_RIGHT_PIN); // success indicator
  } else {
      set_pin(GPIOC, LED_LEFT_PIN);  // failure indicator
  }
  HAL_Delay(2000);
  clear_leds();

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
