#include "DPPTM.h"

/* 目标VDD电压全局变量，由HWCI任务设置，PID任务读取 */
float TargetVDDVoltage = 0.0f;  

/*=================== 实测校准查表（17个精确校准点） ===================*/
/* 索引 = (目标电压 - 2.0) * 10，范围 0~16 对应 2.0V~3.6V
 * 电压:  2.0  2.1  2.2  2.3  2.4  2.5  2.6  2.7  2.8  2.9  3.0  3.1  3.2  3.3  3.4  3.5  3.6
 * 码值: 0x62 0x61 0x60 0x5A 0x59 0x58 0x57 0x56 0x55 0x54 0x53 0x52 0x51 0x50 0x4E 0x4D 0x4C */
static const uint8_t voltage_code_table[17] = {
    0x62, 0x61, 0x60, 0x5A, 0x59, 0x58, 0x57, 0x56,  // 2.0~2.7V
    0x55, 0x54, 0x53, 0x52, 0x51, 0x50, 0x4E, 0x4D, 0x4C  // 2.8~3.6V
};

#define VDD_MIN_VOLTAGE  2.0f                                                   // 最低目标电压
#define VDD_MAX_VOLTAGE  3.6f                                                   // 最高目标电压
#define VDD_STEP         0.1f                                                   // 电压步进
#define VDD_TABLE_SIZE   17                                                     // 查表大小

/**
 * @brief 通过目标电压直接查表获取DPPTM码值
 * @param targetV 目标电压（2.0~3.6V）
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
static uint8_t pid_dpptm      = 0x57;                                          // 当前DPPTM码值（默认2.6V）
static uint8_t pid_baseCode   = 0x57;                                          // 查表基准码值（目标切换时更新）
static float   pid_lastTarget = -1.0f;                                         // 上次目标值
static uint8_t pid_locked     = 0;                                             // 锁定标志

/* PID 微调参数（查表已精确命中，PID仅做1~2步修正） */
static const float PID_KP = 2.0f;                                              // 比例系数（小值，仅微调）
static const float PID_KI = 1.0f;                                              // 积分系数（缓慢消除残差）
static const float PID_KD = 0.3f;                                              // 微分系数（抑制振荡）
static const float PID_INTEGRAL_MAX = 3.0f;                                    // 积分限幅（限制3步）
static const float PID_TOLERANCE = 0.3f;                                      // 容差0.05V（半步精度）
static const int8_t PID_MAX_TRIM = 1;                                          // 最大微调步数（每步=0.1V，±1步已是极限）


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
 *         第二步：PID监控ADC反馈，在基准码值附近3步微调补偿
 *         每次被vADCProcessTask调用执行一步
 */
void SetESLVDDVoltage(void)
{
    /* -------- 0. 前置检查 -------- */
    if (TargetVDDVoltage <= 0.0f)                                               // 目标无效
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

    // PID输出（error>0  需减小码值升压  输出为负）
    float output = -(PID_KP * error + PID_KI * pid_integral + PID_KD * derivative);

    // 转为整数步进（PID输出不足半步时不调整，避免0.1V/步的振荡）
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
    if (newCode < 0x4C) newCode = 0x4C;                                         // 绝对下限（3.6V）
    if (newCode > 0x62) newCode = 0x62;                                         // 绝对上限（2.0V）

    pid_dpptm = (uint8_t)newCode;
    ControlDPPTM(pid_dpptm);                                                   // 写入微调码值
}