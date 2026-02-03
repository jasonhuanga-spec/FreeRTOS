#include "ReceiveDataProcess.h" // 包含解析和队列的声明

/* HWCI 任务句柄定义 */ // 每行注释
TaskHandle_t xHWCIProcessTaskHandle = NULL; // 定义 HWCI 任务句柄

/* 解析后数据队列定义 */ // 每行注释
QueueHandle_t xParsedDataQueue = NULL; // 用于在解析和处理之间传递数据的队列

/**
 * @brief HWCI 处理任务：从解析队列获取 ParsedDataPacket 并执行对应处理
 */
void vHWCIProcessTask(void *pvParameters) // HWCI 任务入口函数
{
    ParsedDataPacket_t localPacket; // 本地变量：存放从队列接收的解析后数据包

    for(;;) // 永久循环，任务主循环
    {
        if (xParsedDataQueue != NULL) // 检查队列是否已创建
        {
            if (xQueueReceive(xParsedDataQueue, &localPacket, portMAX_DELAY) == pdPASS) // 阻塞等待解析数据包
            {
                /* 根据 FunctionCode 分发处理逻辑 */ // 每行注释
                switch (localPacket.FunctionCode) // 根据功能码选择处理分支
                {
                    case 0x02: // 功能码示例：0x02
                        // TODO: 在此处添加对应的处理逻辑
                        if (localPacket.ExecIndex == TASK_ID_SelectESLSPI)
                        {
                            SelectESLSPI(localPacket.Data[0]); // 调用 SelectESLSPI 函数，传入 SPIXLine 参数
                        }
                        
                        QueueSendfmt(xReceiveLogQueue, 0, "HWCI: 处理成功, ExecIndex=%u, DataLen=%u\r\n", localPacket.ExecIndex, localPacket.DataLen); // 发送日志
                        break; // 退出该 case

                    default: // 未知功能码分支处理
                        QueueSendfmt(xReceiveLogQueue, 0, "HWCI: 未知功能码 0x%02X\r\n", localPacket.FunctionCode); // 发送未知功能码日志
                        break; // 退出默认分支
                }
            }
        }
    }
}

/**
 * @brief 创建并启动 HWCI 处理任务和解析结果队列
 */
void vCreateHWCIProcessTask(void) // 创建 HWCI 任务的函数实现
{
    /* 创建解析后数据队列，容量为 10 个 ParsedDataPacket_t */ // 每行注释
    xParsedDataQueue = xQueueCreate(10, sizeof(ParsedDataPacket_t)); // 创建队列用于在解析和处理之间传递数据

    if (xParsedDataQueue == NULL) // 如果队列创建失败
    {
        QueueSendfmt(xReceiveLogQueue, 0, "创建 xParsedDataQueue 失败\r\n"); // 记录队列创建失败日志
        return; // 返回，不创建任务
    }

    /* 创建 HWCI 处理任务并保存句柄 */ // 每行注释
    xTaskCreate(vHWCIProcessTask, 
                "vHWCIProcessTask", 
                256, 
                NULL, 
                tskIDLE_PRIORITY, 
                &xHWCIProcessTaskHandle); // 创建任务并获取任务句柄
}
