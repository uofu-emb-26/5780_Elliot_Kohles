#include <cstdint>
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

void Error_Handler(void);
static void LED_Init(void);
static void ADC_GPIO_Init(void);
static uint8_t ADC1_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
