#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Error_Handler(void);
void INIT_STMHARDWARE(void);
void ESTABLISH_I2C2_COMMS(void);
void READ_WHOAMI(void);
void VERIFY_WHOAMI(uint8_t who_am_i);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
