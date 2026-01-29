#ifndef ADCAPP_H
#define ADCAPP_H

#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "dma.h"

// STM32F103 ADC 相关参数定义
#define ADC_RESOLUTION  4095    // 12位ADC的最大值 (2^12 - 1)
#define ADC_REF_VOLTAGE 3.3f    // ADC参考电压，单位V
// 电路分压系数（196k/(1M+196k)）
#define VOLTAGE_DIV_RATIO 0.164f
// R39/R40均为1kΩ，分压系数1/2
#define LEVEL_SHIFT_RATIO 2.0f



float adc_to_voltage(uint16_t adc_value);
void print_voltage(void);

#endif