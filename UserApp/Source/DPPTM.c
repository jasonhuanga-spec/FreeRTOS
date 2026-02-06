#include "DPPTM.h"


/**
 * @函数名称       电子电位器抽头初始化
 * @说明           Date:是需要调整抽头位置的数据
 */
void ControlDPPTM(uint8_t Data)
{
    Soft_IIC_Start();                                                         //发送IIC起始信号     
    
    if(Soft_IIC_Write_Byte(0x7C))                                             //发送从机地址（0x7C），返回值表示是否发送成功
    {
        #if TEST_MODE
        QueueSendfmt(xReceiveLogQueue, 0, "发送数字电位器地址成功\r\n");
        #endif
        if (Soft_IIC_Write_Byte(0x00))                                        //地址发送成功,发送命令字（0x00）
        {
            #if TEST_MODE
            QueueSendfmt(xReceiveLogQueue, 0, "发送数字电位器写命令成功\r\n");
            #endif
            if (Soft_IIC_Write_Byte(Data))                                    //发送抽头调整数据
            {
                #if TEST_MODE
                QueueSendfmt(xReceiveLogQueue, 0, "向数字电位器写入数据成功,数据 = 0x%02X\r\n", Data);
                #endif
            }
            else
            {
                QueueSendfmt(xReceiveLogQueue, 0, "向数字电位器写入数据失败\r\n");
            }
        }
    }
    
    Soft_IIC_Stop();                                                           //发送IIC停止信号
}


/**
 * @函数名称       设置ESL的VDD电压值
 * @说明           根据索引确定目标电压，通过闭环控制调整数字电位器达到目标值
 *                 data越小电压越大，data越大电压越小，范围0-254
 * @param VDDIndx  电压索引（0-16），对应2.0V-3.6V，步进0.1V
 */
void SetESLVDDVoltage(uint8_t VDDIndx)
{
    float TargetVDDVoltage = 0.0f; // 目标VDD电压值

    // 根据索引设置目标电压
    switch (VDDIndx)
    {
    case 0:
        TargetVDDVoltage = 2.0f; // 索引0对应2.0V
        break;
    case 1:
        TargetVDDVoltage = 2.1f; // 索引1对应2.1V
        break;
    case 2:
        TargetVDDVoltage = 2.2f; // 索引2对应2.2V
        break;
    case 3:
        TargetVDDVoltage = 2.3f; // 索引3对应2.3V
        break;
    case 4:
        TargetVDDVoltage = 2.4f; // 索引4对应2.4V
        break;
    case 5:
        TargetVDDVoltage = 2.5f; // 索引5对应2.5V
        break;
    case 6: 
        TargetVDDVoltage = 2.6f; // 索引6对应2.6V
        break;
    case 7:
        TargetVDDVoltage = 2.7f; // 索引7对应2.7V
        break;
    case 8:
        TargetVDDVoltage = 2.8f; // 索引8对应2.8V
        break;
    case 9:
        TargetVDDVoltage = 2.9f; // 索引9对应2.9V
        break;
    case 10:
        TargetVDDVoltage = 3.0f; // 索引10对应3.0V
        break;
    case 11:
        TargetVDDVoltage = 3.1f; // 索引11对应3.1V
        break;
    case 12:
        TargetVDDVoltage = 3.2f; // 索引12对应3.2V
        break;
    case 13:
        TargetVDDVoltage = 3.3f; // 索引13对应3.3V
        break;
    case 14:
        TargetVDDVoltage = 3.4f; // 索引14对应3.4V
        break;
    case 15:
        TargetVDDVoltage = 3.5f; // 索引15对应3.5V
        break;  
    case 16:
        TargetVDDVoltage = 3.6f; // 索引16对应3.6V
        break;
    default:
        QueueSendfmt(xReceiveLogQueue, 0, "无效的VDD索引: %u\r\n", VDDIndx); // 记录无效索引
        return; // 无效索引，直接返回
    }

    // 闭环控制参数
    const float VOLTAGE_TOLERANCE = 0.00f; // 电压容差0.00V，即允许误差范围
    const uint8_t MAX_ITERATIONS = 50;     // 最大调整次数，防止死循环
    const uint8_t DPPTM_MIN = 0;           // 数字电位器最小值（对应最大电压）
    const uint8_t DPPTM_MAX = 254;         // 数字电位器最大值（对应最小电压）
    
    uint8_t currentDPPTM = 127;            // 初始值设为中间值
    uint8_t iteration = 0;                 // 迭代计数器
    
    QueueSendfmt(xReceiveLogQueue, 0, "开始调整VDD至目标值: %.2fV\r\n", TargetVDDVoltage); // 记录开始调整

    // 闭环控制循环
    while (iteration < MAX_ITERATIONS)
    {
        iteration++; // 迭代次数加1
        
        // 获取当前VDD电压
        float currentVoltage = get_current_vdd_voltage(); // 从ADC获取当前电压
        
        // 检查读取是否成功
        if (currentVoltage < 0.0f)
        {
            QueueSendfmt(xReceiveLogQueue, 0, "ADC读取失败,中止调整\r\n"); // 记录读取失败
            return; // ADC读取失败，退出
        }
        
        // 计算误差
        float error = TargetVDDVoltage - currentVoltage; // 计算目标值与当前值的差
        
        QueueSendfmt(xReceiveLogQueue, 0, "第%u次调整: 当前=%.2fV, 目标=%.2fV, 误差=%.2fV, DPPTM=%u\r\n", 
            iteration, currentVoltage, TargetVDDVoltage, error, currentDPPTM); // 记录调整信息
        
        // 判断是否达到目标（在容差范围内）
        if (error >= -VOLTAGE_TOLERANCE && error <= VOLTAGE_TOLERANCE)
        {
            QueueSendfmt(xReceiveLogQueue, 0, "VDD电压调整成功: %.2fV (目标: %.2fV)\r\n", 
                currentVoltage, TargetVDDVoltage); // 记录调整成功
            return; // 达到目标，退出
        }
        
        // 计算调整量（比例控制）
        // error > 0 表示当前电压低于目标，需要增大电压，即减小DPPTM值
        // error < 0 表示当前电压高于目标，需要减小电压，即增大DPPTM值
        int16_t adjustment = (int16_t)(-error * 30.0f); // 比例系数30（可根据实际情况调整）
        
        // 更新DPPTM值
        int16_t newDPPTM = (int16_t)currentDPPTM + adjustment; // 计算新的DPPTM值
        
        // 边界限制
        if (newDPPTM < DPPTM_MIN)
        {
            newDPPTM = DPPTM_MIN; // 限制在最小值
        }
        if (newDPPTM > DPPTM_MAX)
        {
            newDPPTM = DPPTM_MAX; // 限制在最大值
        }
        
        currentDPPTM = (uint8_t)newDPPTM; // 更新当前DPPTM值
        
        // 控制数字电位器
        ControlDPPTM(currentDPPTM); // 发送新的DPPTM值
        
        // 等待电压稳定
        vTaskDelay(pdMS_TO_TICKS(100)); // 延迟100ms等待电压稳定
    }
    
    // 达到最大迭代次数仍未收敛
    QueueSendfmt(xReceiveLogQueue, 0, "VDD电压调整超时,未能达到目标值\r\n"); // 记录调整超时
}
