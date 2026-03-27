#ifndef ADCAPP_H
#define ADCAPP_H

#include "main.h"
#include "cmsis_os.h" // 引入 CMSIS-OS 头文件，支持 FreeRTOS API
#include "gpio.h"
#include "adc.h"
#include "dma.h"
#include "SendDataProcess.h" // 包含日志发送模块
#include "TaskList.h" // 包含功能码定义（FUNCTION_CODE_DataPacket等）
#include "uart1.h"
#include <string.h> // 用于 memcpy 将浮点数组转为字节流
#include "ESL.h" // 包含 ESL 模块，用于检查 ESL_RESET_Flag 以决定是否发送电压数据包

// STM32F103 ADC 相关参数定义
#define ADC_RESOLUTION  4095    // 12位ADC的最大值 (2^12 - 1)
#define ADC_REF_VOLTAGE 3.3f    // ADC参考电压，单位V
// 电路分压系数（196k/(1M+196k)）
#define VOLTAGE_DIV_RATIO 0.164f
// R39/R40均为1kΩ，分压系数1/2
#define LEVEL_SHIFT_RATIO 2.0f

//ADC分压采集网络的上分压电阻
#define RH 1000000.0f   // 1M
//ADC分压采集网络的下分压电阻
#define RL 196000.0f    // 196k


extern float calibrated_voltage_array[11]; // 全局电压数组，存储转换后的实际电压值（单位V）
extern uint8_t adc_dma_complete; // ADC DMA转换完成标志



uint8_t vADC_StartConversion(void);
float adc_to_voltage(uint16_t adc_value);
void send_voltage_packet(void); /* 发送电压数据包的函数声明 */ 
void send_current_packet(void); /* 发送电流数据包的函数声明 */ 
void vCreateADCTask(void); /* 创建 ADC 处理任务的函数声明 */ 
void vADCProcessTask(void *pvParameters); /* ADC 处理任务的主函数声明 */ 
float adc_voltage_to_current(float adc_voltage); /* 将ADC电压值转换为电流值的函数声明 */ 
void calibrate_channel_voltage(void); /* 所有通道电压校准函数声明 */ 
void calibrate_channel_current(void); /* 所有通道电流校准函数声明 */ 

#endif // ADCAPP_H