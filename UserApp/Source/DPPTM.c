#include "DPPTM.h"

/* 目标VDD电压全局变量，由HWCI任务设置，PID任务读取 */
float TargetVDDVoltage = 3.3f;

/*=================== 实测校准查表（0.0V~3.3V，0.1V步进） ===================*/
/* 索引 = 目标电压 / 0.1，范围 0~33 对应 0.0V~3.3V
 * 端点实测：0x00=0V，0x7F=3.3V
 * 中间点先采用线性标定值，后续可按实测继续修正
 */
static const uint8_t voltage_code_table[34] = {
    0x00, 0x04, 0x08, 0x0C, 0x0F, 0x13, 0x17, 0x1B,  // 0.0~0.7V
    0x1F, 0x23, 0x26, 0x2A, 0x2E, 0x32, 0x36, 0x3A,  // 0.8~1.5V
    0x3E, 0x41, 0x45, 0x49, 0x4D, 0x51, 0x55, 0x59,  // 1.6~2.3V
    0x5C, 0x60, 0x64, 0x68, 0x6C, 0x70, 0x73, 0x77,  // 2.4~3.1V
    0x7B, 0x7F                                                   // 3.2~3.3V
};

#define VDD_MIN_VOLTAGE  0.0f                                                   // 最低目标电压
#define VDD_MAX_VOLTAGE  3.3f                                                   // 最高目标电压
#define VDD_STEP         0.1f                                                   // 电压步进
#define VDD_TABLE_SIZE   34                                                     // 查表大小
#define DPPTM_CODE_MIN   0x00                                                   // 数字电位器码值下限
#define DPPTM_CODE_MAX   0x7F                                                   // 数字电位器码值上限

/**
 * @brief 通过目标电压直接查表获取DPPTM码值
 * @param targetV 目标电压（0.0~3.3V）
 * @return 对应的DPPTM码值
 */
static uint8_t voltage_to_code(float targetV)
{
    if (targetV < VDD_MIN_VOLTAGE) targetV = VDD_MIN_VOLTAGE;                   // 下限钳位
    if (targetV > VDD_MAX_VOLTAGE) targetV = VDD_MAX_VOLTAGE;                   // 上限钳位
    int idx = (int)((targetV - VDD_MIN_VOLTAGE) / VDD_STEP + 0.5f);            // 四舍五入到最近索引
    if (idx < 0) idx = 0;                                                      // 索引下限
    if (idx >= VDD_TABLE_SIZE) idx = VDD_TABLE_SIZE - 1;                        // 索引上限
    return voltage_code_table[idx];                                             // 返回查表码值
}

/*===================== PID 微调状态（模块内部） ====================*/
static float   pid_integral   = 0.0f;                                          // 积分项累积
static float   pid_lastError  = 0.0f;                                          // 上次误差
static uint8_t pid_dpptm      = 0x7F;                                          // 当前DPPTM码值（默认约3.3V）
static uint8_t pid_baseCode   = 0x40;                                          // 查表基准码值（目标切换时更新）
static float   pid_lastTarget = -1.0f;                                         // 上次目标值
static uint8_t pid_locked     = 0;                                             // 锁定标志

/* PID 微调参数（查表已命中，PID仅做小范围码值修正） */
static const float PID_KP = 2.0f;                                              // 比例系数（小值，仅微调）
static const float PID_KI = 1.0f;                                              // 积分系数（缓慢消除残差）
static const float PID_KD = 0.3f;                                              // 微分系数（抑制振荡）
static const float PID_INTEGRAL_MAX = 3.0f;                                    // 积分限幅（限制3步）
static const float PID_TOLERANCE = 0.05f;                                      // 容差0.05V
static const int8_t PID_MAX_TRIM = 4;                                          // 最大微调步数（每步约0.026V，±4步约0.1V）


/**
 * @brief 电子电位器抽头写入
 * @param Data 需要调整的抽头位置数据
 */
void ControlDPPTM(uint8_t Data)
{
    Soft_IIC_Start();
    
    if(Soft_IIC_Write_Byte(0x7C))
    {
        #if TEST_MODE
        QueueSendfmt(xSendDataQueue, 0, "发送数字电位器地址成功\r\n");
        #endif
        if (Soft_IIC_Write_Byte(0x00))
        {
            #if TEST_MODE
            QueueSendfmt(xSendDataQueue, 0, "发送数字电位器写命令成功\r\n");
            #endif
            if (Soft_IIC_Write_Byte(Data))
            {
                #if TEST_MODE
                QueueSendfmt(xSendDataQueue, 0, "向数字电位器写入数据成功,数据 = 0x%02X\r\n", Data);
                #endif
            }
            else
            {
                QueueSendfmt(xSendDataQueue, 0, "向数字电位器写入数据失败\r\n");
            }
        }
    }
    
    Soft_IIC_Stop();
}


/**
 * @brief  查表 + PID微调 调节ESL VDD电压
 * @说明   第一步：目标切换时查表直接命中精确码值（一步到位）
 *         第二步：PID监控ADC反馈，在基准码值附近小范围微调补偿
 *         每次被vADCProcessTask调用执行一步
 */
void SetESLVDDVoltage(void)
{
    /* -------- 0. 前置检查 -------- */
    if (TargetVDDVoltage < 0.0f)                                                // 目标无效
        return;

    float currentV = get_current_vdd_voltage();                                 // 读取当前VDD电压
    if (currentV < 0.0f)                                                       // ADC数据无效
        return;

    /* -------- 1. 目标切换  查表直接命中 -------- */
    if (TargetVDDVoltage != pid_lastTarget)
    {
        pid_integral  = 0.0f;                                                  // 重置积分
        pid_lastError = 0.0f;                                                  // 重置微分
        pid_lastTarget = TargetVDDVoltage;                                     // 记录新目标
        pid_locked = 0;                                                        // 解锁

        pid_baseCode = voltage_to_code(TargetVDDVoltage);                       // 查表获取精确码值
        pid_dpptm = pid_baseCode;                                              // 设置当前码值=基准码值
        ControlDPPTM(pid_dpptm);                                               // 一步写入

        #if TEST_MODE
        QueueSendfmt(xSendDataQueue, 0,
                     "查表命中: %.1fV -> DPPTM=0x%02X(%u)\r\n",
                     TargetVDDVoltage, pid_dpptm, pid_dpptm);
        #endif

        return;                                                                 // 等下一轮ADC采样后再微调
    }

    /* -------- 2. 已锁定  监控漂移 -------- */
    if (pid_locked)
    {
        float drift = TargetVDDVoltage - currentV;
        if (drift >= -PID_TOLERANCE && drift <= PID_TOLERANCE)
            return;                                                            // 仍在容差内，跳过

        pid_locked = 0;                                                        // 漂移超差，解锁微调
        pid_lastError = drift;                                                 // 初始化微分项
    }

    /* -------- 3. PID微调 -------- */
    float error = TargetVDDVoltage - currentV;                                  // 计算误差

    // 在容差内  锁定
    if (error >= -PID_TOLERANCE && error <= PID_TOLERANCE)
    {
        pid_locked = 1;

        #if TEST_MODE
        QueueSendfmt(xSendDataQueue, 0,
                     "PID锁定: %.2fV, DPPTM=0x%02X(%u)\r\n",
                     currentV, pid_dpptm, pid_dpptm);
        #endif
        
        return;
    }

    // PID计算
    pid_integral += error;
    if (pid_integral > PID_INTEGRAL_MAX) pid_integral = PID_INTEGRAL_MAX;
    if (pid_integral < -PID_INTEGRAL_MAX) pid_integral = -PID_INTEGRAL_MAX;

    float derivative = error - pid_lastError;
    pid_lastError = error;

    // PID输出（error>0  需增大码值升压  输出为正）
    float output = (PID_KP * error + PID_KI * pid_integral + PID_KD * derivative);

    // 转为整数步进（PID输出不足半步时不调整，避免抖动）
    int16_t step = 0;
    if (output > 0.5f)       step = (int16_t)(output + 0.5f);                   // 正方向超半步
    else if (output < -0.5f) step = (int16_t)(output - 0.5f);                   // 负方向超半步
    // output在[-0.5, 0.5]之间 → step=0，不调整（查表已精确命中，无需强制步进）

    if (step == 0)                                                             // PID认为无需调整
        return;                                                                // 跳过本轮

    // 限制在基准码值PID_MAX_TRIM步以内（防止PID跑飞）
    int16_t newCode = (int16_t)pid_dpptm + step;
    int16_t lo = (int16_t)pid_baseCode - PID_MAX_TRIM;                          // 微调下限
    int16_t hi = (int16_t)pid_baseCode + PID_MAX_TRIM;                          // 微调上限
    if (newCode < lo) newCode = lo;
    if (newCode > hi) newCode = hi;
    if (newCode < DPPTM_CODE_MIN) newCode = DPPTM_CODE_MIN;                     // 绝对下限（0x00）
    if (newCode > DPPTM_CODE_MAX) newCode = DPPTM_CODE_MAX;                     // 绝对上限（0x7F）

    pid_dpptm = (uint8_t)newCode;
    ControlDPPTM(pid_dpptm);                                                   // 写入微调码值
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
    return calibrated_voltage_array[0]; // 返回VDD通道的电压值
}
