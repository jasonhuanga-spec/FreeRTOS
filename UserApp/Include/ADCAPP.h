#ifndef ADCAPP_H
#define ADCAPP_H

#include "main.h"
#include "cmsis_os.h" // 引入 CMSIS-OS 头文件，支持 FreeRTOS API
#include "gpio.h"
#include "adc.h"
#include "dma.h"
#include "SendDataProcess.h" // 包含日志发送模块
#include "TaskList.h" // 包含功能码定义（FUNCTION_CODE_DataPacket等）
#include "usbd_cdc_if.h" // 包含USB CDC发送函数

// STM32F103 ADC 相关参数定义
#define ADC_RESOLUTION  4095    // 12位ADC的最大值 (2^12 - 1)
#define ADC_REF_VOLTAGE 3.3f    // ADC参考电压，单位V
// 电路分压系数（196k/(1M+196k)）
#define VOLTAGE_DIV_RATIO 0.164f
// R39/R40均为1kΩ，分压系数1/2
#define LEVEL_SHIFT_RATIO 2.0f
// 定义校准偏移量，基于实测数据：
// 实测 0V -> -0.60V
// 实测 3.5V -> 2.92V
// 误差约 -0.6V，故增加 +0.60V 补偿
#define ADC_VOLTAGE_OFFSET 0.60f



float adc_to_voltage(uint16_t adc_value);
void send_voltage_packet(void); /* 发送电压数据包的函数声明 */ // 每行注释
float get_current_vdd_voltage(void); /* 获取当前VDD电压值的函数声明 */ // 每行注释
void vCreateADCTask(void); /* 创建 ADC 处理任务的函数声明 */ // 每行注释
void vADCProcessTask(void *pvParameters); /* ADC 处理任务的主函数声明 */ // 每行注释

#endif