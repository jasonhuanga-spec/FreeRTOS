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

#define ADC_CHANNEL_COUNT 11U // ADC通道数量
#define CURRENT_AVG_WINDOW 64U // 电流滑动平均窗口长度

// 11路电流滑动平均缓存
static float current_sum[ADC_CHANNEL_COUNT] = {0.0f}; // 每路窗口内电流累加和
static float current_hist[ADC_CHANNEL_COUNT][CURRENT_AVG_WINDOW] = {{0.0f}}; // 每路历史电流环形缓存
static uint16_t current_hist_index = 0U; // 当前写入的窗口索引
static uint16_t current_hist_count = 0U; // 当前有效样本数（启动阶段小于窗口长度）

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
        send_current_packet();     // 将电流数据打包发送给上位机

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
 * @brief 电压差分法：由通道电压快速换算电流
 * @param channel_voltage 通道电压（单位V）
 * @return float 电流值（单位mA）
 */
static float channel_voltage_to_current_diff(float channel_voltage)
{
    // 电压差分：相对0A零点偏置做差，减少反推ADC引脚电压的中间步骤
    float voltage_diff = channel_voltage - ADC_VOLTAGE_OFFSET;

    // 与原链路等价：I = (Vch - OFFSET) * DIV_RATIO / (RL/(RH+RL))
    static const float current_gain = (VOLTAGE_DIV_RATIO * (RH + RL) / RL);
    return voltage_diff * current_gain; // 保留正负电流
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
 * @brief 发送电流数据包给上位机
 * @note 将11个ADC通道电流做滑动平均后通过BuildReplyPacket构建数据包发送
 */
void send_current_packet(void)
{
    float current_array[ADC_CHANNEL_COUNT]; // 11路平均电流数据（单位mA）
    uint16_t sample_count = current_hist_count; // 本轮用于平均的有效样本数

    if (sample_count < CURRENT_AVG_WINDOW) // 启动阶段逐步增加样本数
    {
        sample_count++; // 样本数+1
    }

    for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++) // 遍历11个通道
    {
        // 基于voltage_array使用电压差分法计算瞬时电流（减少计算步骤，提升速度）
        float current_inst = channel_voltage_to_current_diff(voltage_array[i]);

        current_sum[i] -= current_hist[i][current_hist_index]; // 移除最旧样本
        current_hist[i][current_hist_index] = current_inst; // 写入当前样本
        current_sum[i] += current_inst; // 加入最新样本

        current_array[i] = current_sum[i] / (float)sample_count; // 得到该通道滑动平均电流
    }

    current_hist_count = sample_count; // 更新有效样本数
    current_hist_index++; // 环形索引前进
    if (current_hist_index >= CURRENT_AVG_WINDOW) // 到窗口末尾后回绕
    {
        current_hist_index = 0U; // 回到起点
    }

    /* 将浮点电流数组按内存布局打包为字节流（小端float，44字节） */
    uint8_t current_data[sizeof(current_array)];
    memcpy(current_data, current_array, sizeof(current_array));

    /* 使用 BuildReplyPacket 构建数据包 */
    uint8_t outBuf[PACKET_MAX_SIZE]; // 输出缓冲区
    uint16_t outLen = 0; // 输出长度

    uint8_t buildRet = BuildReplyPacket((uint8_t)FUNCTION_CODE_DataPacket,
                                        TASK_ID_SendCurrentPacket,
                                        current_data,
                                        (uint8_t)sizeof(current_data),
                                        outBuf,
                                        &outLen); // 构建电流数据包

    if (buildRet != 0) // 检查构建结果
    {
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "构建电流数据包失败，错误码=%u\r\n", buildRet);
        }
        return;
    }

    /* 通过统一发送队列串行化发送，避免与应答/日志并发冲突 */
    if (SendBinaryToHost(outBuf, outLen, pdMS_TO_TICKS(20)) == pdPASS)
    {
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "电流数据包入队成功，长度=%u\r\n", outLen);
        }
    }
    else
    {
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "电流数据包入队失败\r\n");
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


/**
 * @brief 将ADC电压值转换为对应的电流值
 * @param adc_voltage ADC转换得到的电压值（单位V）
 * @return float 返回计算得到的电流值（单位mA），如果输入无效返回-1.0f
 */
float adc_voltage_to_current(float adc_voltage)
{
    // 分压比例
    const float k = RL / (RH + RL);   

    // Vadc = (Vin * k + 3.3) / 2
    // Vin  = (2 * Vadc - 3.3) / k
    return (2.0f * adc_voltage - 3.3f) / k;
}
