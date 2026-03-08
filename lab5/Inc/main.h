#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Error_Handler(void);
void INIT_STMHARDWARE(void);
void ESTABLISH_I2C2_COMMS(void);
uint8_t I2C_READ_REGISTER(uint8_t slave_addr, uint8_t reg);
void I2C_READ_REGISTERS(uint8_t slave_addr, uint8_t reg, uint8_t *buffer, uint16_t length);
// void READ_WHOAMI(void);
// void VERIFY_WHOAMI(uint8_t who_am_i);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
