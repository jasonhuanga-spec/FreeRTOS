#ifndef DPPTM_H
#define DPPTM_H


#include "main.h"
#include "DPPTMIIC.h"
#include "Tasklist.h"


/*函数声明***********************************************************************************************************/

void DPPTMInitial(uint8_t Data);
void DPPTM_RW(uint8_t Address, uint8_t RW, uint8_t DataNum);
void DPPTMRead(uint8_t Address, uint8_t RW, uint8_t DataNum);
void DPPTMWrite(uint8_t Address, uint8_t RW, uint8_t DataNum);

/*函数声明***********************************************************************************************************/



#endif