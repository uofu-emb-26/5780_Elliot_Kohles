#include "main.h"
#include "stm32f0xx_hal.h"
#include <stdio.h>

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

  IO_Pin_Config(); // Configure GPIO pins for USART3
  USART3_Init();   // Initialize USART3
  LED_Init();      // Initialize LEDs PC6-PC9

  while (1)
  {
    char num; // Variable to store the number received after the initial character
    char error_msg[100]; // Buffer to store error messages

    Transmit_String("Enter Command: "); // Prompt the user to enter a command
    while(!(USART3->ISR & USART_ISR_RXNE)); // Wait until a character is received
    
    char c = USART3->RDR; // Read the received character, NOTE: Reading RDR clears the RXNE flag
    Transmit_Character(c); // Echo the received character back to the user

    switch(c) {
      case 'R': // If 'R' is received, turn on the red LED (PC6)
        while(!(USART3->ISR & USART_ISR_RXNE)); // Wait until a character is received
        num = USART3->RDR; // Read the received character, NOTE: Reading RDR clears the RXNE flag

        // Echo the received number back to the user. Newline for clean formatting.
        Transmit_Character(num);
        Transmit_Character('\r\n');

        switch(num) {
          case '0':
            GPIOC->ODR &= ~(1 << 6); // Turn off red LED
            Transmit_String("Red Off\r\n");
            break;
          case '1':
            GPIOC->ODR |= (1 << 6); // Turn on red LED
            Transmit_String("Red On\r\n");
            break;
          case '2':
            GPIOC->ODR ^= (1 << 6); // Toggle red LED
            Transmit_String("Toggle Red\r\n");
            break;
          default:
            sprintf(error_msg, "Invalid Number: '%c'. Only 0, 1, and 2 are accepted for each LED.\r\n", num);
            Transmit_String(error_msg);
        }
        break;

      case 'r':
        while(!(USART3->ISR & USART_ISR_RXNE));
        num = USART3->RDR;

        Transmit_Character(num); // Echo the received number back to the user
        Transmit_Character('\r\n'); // Echo a carriage return for better formatting
        
        switch(num) {
          case '0':
            GPIOC->ODR &= ~(1 << 6); // Turn off red LED
            Transmit_String("Red Off\r\n");
            break;
          case '1':
            GPIOC->ODR |= (1 << 6); // Turn on red LED
            Transmit_String("Red On\r\n");
            break;
          case '2':
            GPIOC->ODR ^= (1 << 6); // Toggle red LED
            Transmit_String("Toggle Red\r\n");
            break;
          default:
            sprintf(error_msg, "Invalid Number: '%c'. Only 0, 1, and 2 are accepted for each LED.\r\n", num);
            Transmit_String(error_msg);
        }
        break;
      case 'B': // If 'B' is received, turn on the blue LED (PC7)
        while(!(USART3->ISR & USART_ISR_RXNE)); // Wait until a character is received
        num = USART3->RDR; // Read the received character, NOTE: Reading RDR clears the RXNE flag
        Transmit_Character(num); // Echo the received number back to the user
        Transmit_Character('\r\n'); // Echo a carriage return for better formatting
        switch(num) {
          // Case 0 turns off the blue LED, case 1 turns on the blue LED, and case 2 toggles the blue LED
          case '0':
            GPIOC->ODR &= ~(1 << 7); // Turn off blue LED
            Transmit_String("Blue Off\r\n");
            break;
          case '1':
            GPIOC->ODR |= (1 << 7); // Turn on blue LED
            Transmit_String("Blue On\r\n");
            break;
          case '2':
            GPIOC->ODR ^= (1 << 7); // Toggle blue LED
            Transmit_String("Toggle Blue\r\n");
            break;
          default:
            sprintf(error_msg, "Invalid Number: '%c'. Only 0, 1, and 2 are accepted for each LED.\r\n", num);
            Transmit_String(error_msg);
        }
        break;
      case 'b': // If 'b' is received, turn on the blue LED (PC7)
        while(!(USART3->ISR & USART_ISR_RXNE)); // Wait until a character is received
        num = USART3->RDR; // Read the received character, NOTE: Reading RDR clears the RXNE flag
        Transmit_Character(num); // Echo the received number back to the user
        Transmit_Character('\r\n'); // Echo a carriage return for better formatting
        switch(num) {
          // Case 0 turns off the blue LED, case 1 turns on the blue LED, and case 2 toggles the blue LED
          case '0':
            GPIOC->ODR &= ~(1 << 7); // Turn off blue LED
            Transmit_String("Blue Off\r\n");
            break;
          case '1':
            GPIOC->ODR |= (1 << 7); // Turn on blue LED
            Transmit_String("Blue On\r\n");
            break;
          case '2':
            GPIOC->ODR ^= (1 << 7); // Toggle blue LED
            Transmit_String("Toggle Blue\r\n");
            break;
          default:
            sprintf(error_msg, "Invalid Number: '%c'. Only 0, 1, and 2 are accepted for each LED.\r\n", num);
            Transmit_String(error_msg);
        }
      case 'O': // If 'O' is received, turn on the orange LED (PC8)
        while(!(USART3->ISR & USART_ISR_RXNE)); // Wait until a character is received
        num = USART3->RDR; // Read the received character, NOTE: Reading RDR clears the RXNE flag
        Transmit_Character(num); // Echo the received number back to the user
        Transmit_Character('\r\n'); // Echo a carriage return for better formatting
        switch(num) {
          // Case 0 turns off the orange LED, case 1 turns on the orange LED, and case 2 toggles the orange LED
          case '0':
            GPIOC->ODR &= ~(1 << 8); // Turn off orange LED
            Transmit_String("Orange Off\r\n");
            break;
          case '1':
            GPIOC->ODR |= (1 << 8); // Turn on orange LED
            Transmit_String("Orange On\r\n");
            break;
          case '2':
            GPIOC->ODR ^= (1 << 8); // Toggle orange LED
            Transmit_String("Toggle Orange\r\n");
            break;
          default:
            sprintf(error_msg, "Invalid Number: '%c'. Only 0, 1, and 2 are accepted for each LED.\r\n", num);
            Transmit_String(error_msg);
        }
        break;
      case 'o': // If 'o' is received, turn on the orange LED (PC8)
        while(!(USART3->ISR & USART_ISR_RXNE)); // Wait until a character is received
        num = USART3->RDR; // Read the received character, NOTE: Reading RDR clears the RXNE flag
        Transmit_Character(num); // Echo the received number back to the user
        Transmit_Character('\r\n'); // Echo a carriage return for better formatting
        switch(num) {
          // Case 0 turns off the orange LED, case 1 turns on the orange LED, and case 2 toggles the orange LED
          case '0':
            GPIOC->ODR &= ~(1 << 8); // Turn off orange LED
            Transmit_String("Orange Off\r\n");
            break;
          case '1':
            GPIOC->ODR |= (1 << 8); // Turn on orange LED
            Transmit_String("Orange On\r\n");
            break;
          case '2':
            GPIOC->ODR ^= (1 << 8); // Toggle orange LED
            Transmit_String("Toggle Orange\r\n");
            break;
          default:
            sprintf(error_msg, "Invalid Number: '%c'. Only 0, 1, and 2 are accepted for each LED.\r\n", num);
            Transmit_String(error_msg);
        }
      case 'G': // If 'G' is received, turn on the green LED (PC9)
        while(!(USART3->ISR & USART_ISR_RXNE)); // Wait until a character is received
        num = USART3->RDR; // Read the received character, NOTE: Reading RDR clears the RXNE flag
        Transmit_Character(num); // Echo the received number back to the user
        Transmit_Character('\r\n'); // Echo a carriage return for better formatting
        switch(num) {
          // Case 0 turns off the green LED, case 1 turns on the green LED, and case 2 toggles the green LED
          case '0':
            GPIOC->ODR &= ~(1 << 9); // Turn off green LED
            Transmit_String("Green Off\r\n");
            break;
          case '1':
            GPIOC->ODR |= (1 << 9); // Turn on green LED
            Transmit_String("Green On\r\n");
            break;
          case '2':
            GPIOC->ODR ^= (1 << 9); // Toggle green LED
            Transmit_String("Toggle Green\r\n");
            break;
          default:
            sprintf(error_msg, "Invalid Number: '%c'. Only 0, 1, and 2 are accepted for each LED.\r\n", num);
            Transmit_String(error_msg);
        }
        break;
      case 'g': // If 'g' is received, turn on the green LED (PC9)
        while(!(USART3->ISR & USART_ISR_RXNE)); // Wait until a character is received
        num = USART3->RDR; // Read the received character, NOTE: Reading RDR clears the RXNE flag
        Transmit_Character(num); // Echo the received number back to the user
        Transmit_Character('\r\n'); // Echo a carriage return for better formatting
        switch(num) {
          // Case 0 turns off the green LED, case 1 turns on the green LED, and case 2 toggles the green LED
          case '0':
            GPIOC->ODR &= ~(1 << 9); // Turn off green LED
            Transmit_String("Green Off\r\n");
            break;
          case '1':
            GPIOC->ODR |= (1 << 9); // Turn on green LED
            Transmit_String("Green On\r\n");
            break;
          case '2':
            GPIOC->ODR ^= (1 << 9); // Toggle green LED
            Transmit_String("Toggle Green\r\n");
            break;
          default:
            sprintf(error_msg, "Invalid Number: '%c'. Only 0, 1, and 2 are accepted for each LED.\r\n", num);
            Transmit_String(error_msg);
        }
      default: // For any other character, turn off all LEDs
        sprintf(error_msg, "Invalid Character: '%c'. Only r,g,b,o(0-2) are accepted.(example: R1, o0)\r\n", c);
        Transmit_String(error_msg);
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

// Configure GPIO pins for USART3 (PB10 for TX and PB11 for RX)
void IO_Pin_Config(void) {
  // Configure GPIO pins TX and RX for PB10 and PB11 to alternate function mode.
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // Enable clock for GPIOB
  GPIOB->MODER &= ~((3 << (10 * 2)) | (3 << (11 * 2))); // Clear mode bits for PB10 and PB11
  GPIOB->MODER |= (2 << (10 * 2)) | (2 << (11 * 2)); // Set mode to alternate function for PB10 and PB11

  GPIOB->AFR[1] &= ~((0xF << ((10 - 8) * 4)) | (0xF << ((11 - 8) * 4))); // Clear alternate function bits for PB10 and PB11
  GPIOB->AFR[1] |= (4 << ((10 - 8) * 4)) | (4 << ((11 - 8) * 4)); // Set alternate function to AF4 (USART3) for PB10 and PB11
}

// Initialize USART3 with baud rate 115200, 8 data bits, no parity, and 1 stop bit
void USART3_Init(void) {
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN; // Enable clock for USART3 
  USART3->BRR = 8000000 / 115200; // Set baud rate to 115200
  USART3->CR1 |= USART_CR1_TE | USART_CR1_RE; // Enable transmitter and receiver
  USART3->CR1 |= USART_CR1_UE; // Enable USART3
}

void LED_Init(void) {
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  GPIOC->MODER |= (1 << (6 * 2)) // Set PC6, PC7, PC8, and PC9 to output mode
                | (1 << (7*2)) 
                | (1 << (8*2)) 
                | (1 << (9*2));
}

// Transmit a single character over USART3
void Transmit_Character(char c) {
  while(!(USART3->ISR & USART_ISR_TXE)); // Wait until transmit data register is empty
  USART3->TDR = c; // Transmit the character
}

// Transmit a string over USART3
void Transmit_String(char *s) {
  while(*s) {
    Transmit_Character(*s++); // Transmit each character in the string
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
