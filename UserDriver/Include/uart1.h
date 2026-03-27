#ifndef UART1_H
#define UART1_H

#include <stdint.h>

void UART1_Init(void);
void UART1_SendBlocking(const uint8_t *data, uint16_t len);

#endif /* UART1_H */
