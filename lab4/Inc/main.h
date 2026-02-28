#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

void Error_Handler(void);
void Transmit_Character(char c);
void Transmit_String(char *s);
void IO_Pin_Config(void);
void USART3_Init(void);
void LED_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
