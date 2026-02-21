#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

void Error_Handler(void);
void TIM3_PWM_Init(void);
void LED_GPIO_Init(void);
void TIM2_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
