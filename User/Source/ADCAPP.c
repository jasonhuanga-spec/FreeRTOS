#include "ADCAPP.h"


// 1. 改为全局变量（避免局部变量随机值）+ 类型匹配DMA的uint32_t
uint32_t adcbuf[11] = {0}; 
// 2. 添加DMA传输完成标记（核心！解决脏读）
uint8_t adc_dma_complete = 0;



// DMA传输完成回调函数（CubeMX配置了DMA中断才会触发）
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if (hadc->Instance == ADC1)
  {
    adc_dma_complete = 1; // 标记数据传输完成
  }
}


/**
 * @brief 将ADC采样值转换为实际输入电压
 * @param adc_value: ADC采集到的数字值（0~4095）
 * @return 实际输入电压值，单位V（范围约-20.12~+20.12V）
 */
float adc_to_voltage(uint16_t adc_value)
{
    // 1. 先将ADC数值转换为ADC引脚的电压（ADC_VSNL2）
    float adc_pin_voltage = (float)adc_value * ADC_REF_VOLTAGE / ADC_RESOLUTION;
    
    // 2. 反向计算分压后的电压（V_div）
    float v_div = (adc_pin_voltage * LEVEL_SHIFT_RATIO) - ADC_REF_VOLTAGE;
    
    // 3. 反向计算原始输入电压（VSNL2）
    float input_voltage = v_div / VOLTAGE_DIV_RATIO;
    
    return input_voltage;
}

/**
 * @brief 打印ADC值对应的实际电压
 * @param adc_value: ADC采集到的数字值（0~4095）
 */
void print_voltage(void)
{
    // 4. 重置DMA完成标记
  adc_dma_complete = 0;
  
  // 5. 启动ADC DMA采样（校准只需在初始化时执行一次，此处注释）
  // HAL_ADCEx_Calibration_Start(&hadc1); // 移到main函数初始化阶段
  HAL_ADC_Start_DMA(&hadc1, adcbuf, 11); // 无需强制类型转换（adcbuf已是uint32_t）
  
  // 6. 等待DMA传输完成（核心！避免脏读）
  while (adc_dma_complete == 0);
  
  // 7. 停止ADC DMA（避免影响下一次采样）
  HAL_ADC_Stop_DMA(&hadc1);
  
  // 8. 循环打印所有通道
  // for (uint8_t i = 0; i < 11; i++)
  // {
  //   printf("ADC channal : %d ;voltage = %.2f V \r\n", i, adc_to_voltage(adcbuf[i]));
  // }
  printf("VDD voltage = %.2f V \r\n", adc_to_voltage(adcbuf[0]));
  printf("VSPL voltage = %.2f V \r\n", adc_to_voltage(adcbuf[2]));
  printf("VDD15V voltage = %.2f V \r\n", adc_to_voltage(adcbuf[4]));
  printf("VCOM voltage = %.2f V \r\n", adc_to_voltage(adcbuf[5]));
  printf("VGN voltage = %.2f V \r\n", adc_to_voltage(adcbuf[6]));
  printf("VSN voltage = %.2f V \r\n", adc_to_voltage(adcbuf[9]));
  printf("VGP voltage = %.2f V \r\n", adc_to_voltage(adcbuf[10]));
  printf("VSP voltage = %.2f V \r\n", adc_to_voltage(adcbuf[7]));
}
