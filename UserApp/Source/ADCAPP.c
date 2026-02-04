#include "ADCAPP.h"
#include "SendDataProcess.h" // 包含日志发送模块


// 1. 改为全局变量（避免局部变量随机值）+ 类型匹配DMA的uint32_t
uint32_t adcbuf[11] = {0}; 

// 2. 添加DMA传输完成标记（核心！解决脏读）
uint8_t adc_dma_complete = 0;

/* ADC 任务句柄定义 */ // 每行注释
TaskHandle_t xADCProcessTaskHandle = NULL; // 定义 ADC 任务句柄

/**
 * @brief 创建并启动 ADC 处理任务
 */
void vCreateADCTask(void) // 创建 ADC 任务
{
    /* 创建 ADC 处理任务 */ // 每行注释
    xTaskCreate(vADCProcessTask,           /* 任务函数 */
                "vADCProcessTask",         /* 任务名称 */
                512,                       /* 堆栈大小（字） */
                NULL,                      /* 任务参数 */
                tskIDLE_PRIORITY + 1,      /* 优先级 */
                &xADCProcessTaskHandle);   /* 任务句柄 */
}

/**
 * @brief ADC 处理任务主循环
 * @param pvParameters 任务参数（未使用）
 */
void vADCProcessTask(void *pvParameters) // ADC 任务实现
{
    /* 避免编译器警告未使用参数 */ // 每行注释
    (void)pvParameters;

    for (;;) // 任务主循环
    {
        print_voltage(); // 执行 ADC 采样与打印
        
        /* 延迟 1000ms（根据需求调整采样频率） */ // 每行注释
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


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
float adc_to_voltage(uint16_t adc_value) // 转换函数实现
{
    float adc_voltage = (float)adc_value * ADC_REF_VOLTAGE / ADC_RESOLUTION; // 计算引脚电压
    
    // 原始公式：V_in = (V_pin * LEVEL_SHIFT - V_ref) / DIV_RATIO
    float raw_voltage = ((adc_voltage * LEVEL_SHIFT_RATIO) - ADC_REF_VOLTAGE) / VOLTAGE_DIV_RATIO;
    
    // 应用线性补偿
    return raw_voltage + ADC_VOLTAGE_OFFSET; 
}

/**
 * @brief 打印ADC值对应的实际电压
 * @note 使用 HAL_ADC_Start_DMA 并在中断中设置标志位，此处等待完成
 */
void print_voltage(void) // 采样与打印函数实现
{
    adc_dma_complete = 0; // 清除完成标志
    
    /* 启动 DMA 转换，采样 11 个通道到 adcbuf */
    if (HAL_ADC_Start_DMA(&hadc1, adcbuf, 11) != HAL_OK) // 启动 DMA
    {
        /* 启动失败处理，打印错误日志 */
        if (xReceiveLogQueue != NULL) { // 检查日志队列有效性
             QueueSendfmt(xReceiveLogQueue, 0, "ADC Start DMA Error\r\n"); // 发送错误信息
        }
        return; // 退出
    }

    /* 等待转换完成，增加超时机制（10ms足够11个通道转换） */
    uint32_t timeout = 10; // 超时计数 10ms
    while (adc_dma_complete == 0 && timeout > 0) // 等待完成标志或超时
    {
        vTaskDelay(pdMS_TO_TICKS(1)); // 挂起 1ms 避免独占 CPU
        timeout--; // 递减计数
    }
    
    HAL_ADC_Stop_DMA(&hadc1); // 停止 ADC DMA

    if (timeout == 0) // 检查是否超时
    {
        if (xReceiveLogQueue != NULL) { // 检查日志队列有效性
            QueueSendfmt(xReceiveLogQueue, 0, "ADC DMA Timeout\r\n"); // 发送超时信息
        }
        return; // 退出
    }

    /* 格式化并通过日志队列打印各通道电压，格式模仿用户Log */
    if (xReceiveLogQueue != NULL) { // 检查队列有效性
        // 第一行：VDD, VSPL, VDD15V
        QueueSendfmt(xReceiveLogQueue, 0, "VDD:%.2fV VSPL:%.2fV VDD15V:%.2fV\r\n", 
            adc_to_voltage(adcbuf[0]), adc_to_voltage(adcbuf[2]), adc_to_voltage(adcbuf[4])); 
        // 第二行：VCOM, VGN, VSN
        QueueSendfmt(xReceiveLogQueue, 0, "VCOM:%.2fV VGN:%.2fV VSN:%.2fV\r\n", 
            adc_to_voltage(adcbuf[5]), adc_to_voltage(adcbuf[6]), adc_to_voltage(adcbuf[9])); 
        // 第三行：VGP, VSP
        QueueSendfmt(xReceiveLogQueue, 0, "VGP:%.2fV VSP:%.2fV\r\n", 
            adc_to_voltage(adcbuf[10]), adc_to_voltage(adcbuf[7])); 
    }
}
