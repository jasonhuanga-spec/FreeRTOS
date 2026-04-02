#ifndef ESL_H
#define ESL_H


#include "main.h"
#include "ESLComDri.h"
#include "ESL4SPI.h"
#include "SendDataProcess.h"
#include "TaskList.h" // 包含功能码定义（FUNCTION_CODE_ESLCommands等）


extern uint8_t ESL_RESET_Flag; /* ESL复位标志，外部声明供其他模块引用 */


void ESLtest(void);
void E52bitRead(uint8_t Address, uint8_t Number);
void E52bitWrite(uint8_t Address, uint8_t Number, const uint8_t *writeData, uint8_t writeLen);
void ESLCommands(uint16_t execIndex, uint8_t Address, uint8_t RW, uint8_t Number);

#endif          
