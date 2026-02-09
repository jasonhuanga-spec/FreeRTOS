#include "ADCAPP.h"
#include "SendDataProcess.h" // 包含日志发送模块
#include "TaskList.h" // 包含功能码定义（FUNCTION_CODE_DataPacket等）
#include "usbd_cdc_if.h" // 包含USB CDC发送函数
#include <string.h> // 用于 memcpy 将浮点数组转为字节流


// 1. 改为全局变量（避免局部变量随机值）+ 类型匹配DMA的uint32_t
uint32_t adcbuf[11] = {0}; 

// 2. 添加DMA传输完成标记（核心！解决脏读）
uint8_t adc_dma_complete = 0;

// 3. 全局电压数组，存储转换后的实际电压值（单位V）
float voltage_array[11] = {0.0f};

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
                256,                       /* 堆栈大小（字） */
                NULL,                      /* 任务参数 */
                tskIDLE_PRIORITY,      /* 优先级 */
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
        vADC_StartConversion();    // 启动ADC DMA采样并等待完成
        SetESLVDDVoltage();        // PID单步调节VDD电压（利用刚采样的数据）
        send_voltage_packet();     // 将电压数据打包发送给上位机

        vTaskDelay(pdMS_TO_TICKS(10)); // 延迟10ms等待电压稳定（PID控制周期）
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
 * @brief 启动ADC DMA转换并等待完成
 * @return uint8_t 返回转换状态：0表示成功，非0表示失败
 */
uint8_t vADC_StartConversion(void) // ADC DMA转换函数
{
    adc_dma_complete = 0; // 清除完成标志
    
    /* 启动 DMA 转换，采样 11 个通道到 adcbuf */
    if (HAL_ADC_Start_DMA(&hadc1, adcbuf, 11) != HAL_OK) // 启动 DMA
    {
        if (xSendDataQueue != NULL) { // 检查发送队列有效性
            QueueSendfmt(xSendDataQueue, 0, "ADC Start DMA Error\r\n"); // 发送错误信息
        }
        return 1; // 返回错误
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
        if (xSendDataQueue != NULL) { // 检查发送队列有效性
            QueueSendfmt(xSendDataQueue, 0, "ADC DMA Timeout\r\n"); // 发送超时信息
        }
        return 2; // 返回超时错误
    }
    
    return 0; // 返回成功
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
 * @brief 发送电压数据包给上位机
 * @note 将11个ADC通道的电压值转换后通过BuildReplyPacket构建数据包发送
 */
void send_voltage_packet(void) // 发送电压数据包的实现
{
    /* 创建电压值数组，存储转换后的实际电压值（单位：V，使用float转为uint16_t，精度0.01V） */
    for (uint8_t i = 0; i < 11; i++) // 遍历11个通道
    {
        voltage_array[i] = adc_to_voltage(adcbuf[i]); // 将ADC原始值转换为实际电压
    }

    /* 直接将浮点电压数组按内存布局打包为字节流（小端float，44字节） */
    uint8_t voltage_data[sizeof(voltage_array)];
    memcpy(voltage_data, voltage_array, sizeof(voltage_array));

    /* 使用 BuildReplyPacket 构建数据包 */
    uint8_t outBuf[PACKET_MAX_SIZE]; // 输出缓冲区
    uint16_t outLen = 0; // 输出长度
    
    // 功能码使用 FUNCTION_CODE_DataPacket(0x02)，execIndex可自定义（这里用0表示电压数据）
    uint8_t buildRet = BuildReplyPacket((uint8_t)FUNCTION_CODE_DataPacket, TASK_ID_SendVoltagePacket, voltage_data, sizeof(voltage_data), outBuf, &outLen); // 构建数据包
    
    if (buildRet != 0) // 检查构建结果
    {
        if (xSendDataQueue != NULL) { // 若队列存在则记录错误
            QueueSendfmt(xSendDataQueue, 0, "构建电压数据包失败，错误码=%u\r\n", buildRet); // 记录失败
        }
        return; // 构建失败，返回
    }

    /* 通过统一发送队列串行化发送，避免与应答/日志并发冲突 */
    if (SendBinaryToHost(outBuf, outLen, pdMS_TO_TICKS(20)) == pdPASS)
    {
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "电压数据包入队成功，长度=%u\r\n", outLen);
        }
    }
    else
    {
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "电压数据包入队失败\r\n");
        }
    }
}


/**
 * @brief 获取当前VDD电压值
 * @return float 返回当前VDD电压值（单位V），如果数据无效返回-1.0f
 */
float get_current_vdd_voltage(void) // 获取当前VDD电压
{
    // 检查DMA传输是否完成
    if (adc_dma_complete == 0) // 如果DMA未完成
    {
        return -1.0f; // 返回错误值
    }
    
    // 返回voltage_array[0]（VDD电压）
    return voltage_array[0]; // 返回VDD通道的电压值
}
