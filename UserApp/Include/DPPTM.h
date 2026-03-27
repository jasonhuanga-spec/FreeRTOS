#ifndef DPPTM_H
#define DPPTM_H


#include "main.h"
#include "DPPTMIIC.h"
#include "Tasklist.h"
#include "ADCAPP.h" // 包含ADC模块，用于获取当前VDD电压

extern float TargetVDDVoltage;

/*函数声明***********************************************************************************************************/

void ControlDPPTM(uint8_t Data);
void SetESLVDDVoltage(void);
float get_current_vdd_voltage(void); /* 获取当前VDD电压的函数声明 */ // 每行注释


/*函数声明***********************************************************************************************************/



#endif