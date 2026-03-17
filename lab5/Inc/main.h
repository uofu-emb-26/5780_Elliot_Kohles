#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Error_Handler(void);
void delay_init(void);
void delay_ms(uint32_t ms);
void GPIO_init(void);
void I2C_init(void);
void show_direction(int16_t x, int16_t y);
int gyro_read_whoami(uint8_t *whoami);
int gyro_init(void);
int gyro_read_axes(int16_t *x, int16_t *y);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
