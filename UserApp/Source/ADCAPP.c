#include "ADCAPP.h"


#define ADC_CHANNEL_COUNT 11U // ADC通道数量
#define CURRENT_AVG_WINDOW 64U // 电流滑动平均窗口长度
#define OFFSET_SAMPLE_COUNT 30U // 校准采样次数

// 1. 改为全局变量（避免局部变量随机值）+ 类型匹配DMA的uint32_t
uint32_t adcbuf[11] = {0}; 

// 2. 添加DMA传输完成标记（核心！解决脏读）
uint8_t adc_dma_complete = 0;

// 3. 全局电压数组，存储转换后的实际电压值（单位V）
float voltage_array[11] = {0.0f};
// 4. 全局电流数组，存储转换后的实际电流值（单位mA）
float current_array[11] = {0.0f};

// 全局校准后电压数组，存储每通道校准后的电压值（单位V）
float calibrated_voltage_array[11] = {0.0f};
// 每通道校准偏置（默认0，可按通道写入实际标定值）
static float channel_voltage_offset[ADC_CHANNEL_COUNT] = {0.0f};

// 全局校准后电流数组，存储每通道校准后的电流值（单位mA）
float calibrated_current_array[11] = {0.0f};
// 每通道电流偏置（在VDD打开后采样一次作为0mA基线）
static float channel_current_offset[ADC_CHANNEL_COUNT] = {0.0f};

// 用于存储电流偏置的中间变量
float current_offset_sum[ADC_CHANNEL_COUNT] = {0.0f};
// 用于记录采样次数
uint8_t current_offset_count = 0;
// 标记电流偏置是否已准备就绪
uint8_t current_offset_ready = 0;

// 用于存储电压偏置的中间变量
float voltage_offset_sum[ADC_CHANNEL_COUNT] = {0.0f};
// 用于记录电压偏置采样次数
uint8_t voltage_offset_count = 0;
// 标记电压偏置是否已准备就绪
uint8_t voltage_offset_ready = 0;

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
    
    // 不使用固定静态偏移，直接返回物理换算值。
    return raw_voltage;
}

/**
 * @brief 电压差分法：由通道电压快速换算电流
 * @param channel_voltage 通道电压（单位V）
 * @return float 电流值（单位mA）
 */
static float channel_voltage_to_current_diff(float channel_voltage)
{
    // 当前口径不使用固定静态偏移，直接使用通道电压差分。
    // float voltage_diff = channel_voltage;

    // // 与原链路等价：I = (Vch - OFFSET) * DIV_RATIO / (RL/(RH+RL))
    // static const float current_gain = (VOLTAGE_DIV_RATIO * (RH + RL) / RL);
    return (channel_voltage / RL) * 1000.0f; // 保留正负电流，单位mA
}



/**
 * @brief 发送电压数据包给上位机
 * @note 将11个ADC通道的电压值转换后通过BuildReplyPacket构建数据包发送
 */
void send_voltage_packet(void) // 发送电压数据包的实现
{
    // 校准每个通道的电压（更新全局校准数组
    calibrate_channel_voltage(); 

    // 每轮都基于最新ADC值更新“原始/校准后”电压，避免发送陈旧校准数组。
    for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++)
    {
        float raw_voltage = adc_to_voltage((uint16_t)adcbuf[i]);
        voltage_array[i] = raw_voltage;
        calibrated_voltage_array[i] = raw_voltage - channel_voltage_offset[i];
    }

    /* 直接将浮点电压数组按内存布局打包为字节流（小端float，44字节） */
    uint8_t voltage_data[sizeof(calibrated_voltage_array)];
    memcpy(voltage_data, calibrated_voltage_array, sizeof(calibrated_voltage_array));

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
        #if TEST_MODE
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "电压数据包入队成功，长度=%u\r\n", outLen);
        }
        #endif
    }
    else
    {
        // 队列满/阻塞时直接串口兜底发送，保证上位机可收到电压包。
        UART1_SendBlocking(outBuf, outLen);
    }
}


/**
 * @brief 发送电流数据包给上位机
 * @note 将11个ADC通道电流做滑动平均后通过BuildReplyPacket构建数据包发送
 */
void send_current_packet(void)
{
    // 校准每个通道的电流（更新全局校准数组）
    calibrate_channel_current(); 
    // 每轮都基于最新ADC值更新“原始/校准后”电流，避免发送陈旧校准数组。
    for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++)
    {
        // 1) 用“校准后电压”换算瞬时电流，保证电压/电流口径一致。
        float raw_current = channel_voltage_to_current_diff(calibrated_voltage_array[i]);
        current_array[i] = raw_current;
        if (current_offset_ready) {
            calibrated_current_array[i] = current_array[i] - channel_current_offset[i];
        } else {
            calibrated_current_array[i] = current_array[i];
        }
    }


    /* 将校准后电流数组按内存布局打包为字节流（小端float，44字节） */
    uint8_t current_data[sizeof(calibrated_current_array)];
    memcpy(current_data, calibrated_current_array, sizeof(calibrated_current_array));

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
        #if TEST_MODE
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "电流数据包入队成功，长度=%u\r\n", outLen);
        }
        #endif
    }
    else
    {
        UART1_SendBlocking(outBuf, outLen);
    }
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



/**
 * @brief 校准每个通道的电流
 */
void calibrate_channel_current(void)
{
    if ((HAL_GPIO_ReadPin(VDDSwitchGPIOX, VDDSwitchPINX) == GPIO_PIN_RESET) &&
        (voltage_offset_ready == 1) &&
        (current_offset_ready == 0))
    {
        osDelay(500);

        for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++)
       {
            current_offset_sum[i] = 0.0f;
        }

        float current_min[ADC_CHANNEL_COUNT];
        float current_max[ADC_CHANNEL_COUNT];
        for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++)
        {
            current_min[i] = 1e9f;
            current_max[i] = -1e9f;
        }

        for (uint8_t k = 0; k < OFFSET_SAMPLE_COUNT; k++)
        {
            if (vADC_StartConversion() != 0U)
            {
                return;
            }

            for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++)
            {
                float raw_voltage = adc_to_voltage((uint16_t)adcbuf[i]);
                float channel_voltage = raw_voltage - channel_voltage_offset[i];
                float raw_current = channel_voltage_to_current_diff(channel_voltage);

                current_offset_sum[i] += raw_current;
                if (raw_current < current_min[i]) current_min[i] = raw_current;
                if (raw_current > current_max[i]) current_max[i] = raw_current;
            }

            osDelay(5);
        }

        for (uint8_t j = 0; j < ADC_CHANNEL_COUNT; j++)
        {
            if (OFFSET_SAMPLE_COUNT > 2U)
            {
                channel_current_offset[j] =
                    (current_offset_sum[j] - current_min[j] - current_max[j]) /
                    (float)(OFFSET_SAMPLE_COUNT - 2U);
            }
            else
            {
                channel_current_offset[j] =
                    current_offset_sum[j] / (float)OFFSET_SAMPLE_COUNT;
            }
        }

        current_offset_ready = 1;
    }
}


/**
 * @brief 校准每个通道的电压
 */
void calibrate_channel_voltage(void)
{
    if (HAL_GPIO_ReadPin(VDDSwitchGPIOX, VDDSwitchPINX) == GPIO_PIN_SET)
    {
        osDelay(500); // 等待VDD稳定，确保采样数据可靠
        for (uint8_t k = 0; k < OFFSET_SAMPLE_COUNT; k++)
        {
            for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++)
            {
                float raw_voltage = adc_to_voltage((uint16_t)adcbuf[i]);
                channel_voltage_offset[i] = raw_voltage + channel_voltage_offset[i];
            }
        }
        
        for (uint8_t j = 0; j < ADC_CHANNEL_COUNT; j++)
        {
            channel_voltage_offset[j] = channel_voltage_offset[j] / (float)OFFSET_SAMPLE_COUNT;
        }
        
        voltage_offset_ready = 1;// 电压偏置准备就绪
        VDDSwitch(VDDON); // 校准完成后打开VDD，准备校准电流偏置
    }    
}

