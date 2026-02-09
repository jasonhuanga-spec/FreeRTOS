#ifndef ESL_H
#define ESL_H


#include "main.h"
#include "ESLComDri.h"
#include "ESL4SPI.h"
#include "SendDataProcess.h"
#include "TaskList.h" // 包含功能码定义（FUNCTION_CODE_ESLCommands等）



void ESLtest(void);
void E52bitRead(uint8_t Address, uint8_t Number);
void ESLCommands(uint8_t Address, uint8_t RW, uint8_t Number);

#endif          