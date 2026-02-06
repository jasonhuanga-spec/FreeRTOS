#ifndef DPPTM_H
#define DPPTM_H


#include "main.h"
#include "DPPTMIIC.h"
#include "Tasklist.h"
#include "ADCAPP.h" // 包含ADC模块，用于获取当前VDD电压


/*函数声明***********************************************************************************************************/

void ControlDPPTM(uint8_t Data);
void SetESLVDDVoltage(uint8_t VDDIndx);

/*函数声明***********************************************************************************************************/



#endif