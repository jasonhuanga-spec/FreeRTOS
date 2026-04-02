#include "TaskList.h" // 包含解析和队列的声明

#define REGISTER_WRITE_TRANSACTION_TIMEOUT_MS 3000U

/* HWCI 任务句柄定义 */ // 每行注释
TaskHandle_t xHWCIProcessTaskHandle = NULL; // 定义 HWCI 任务句柄

/* 解析后数据队列定义 */ // 每行注释
QueueHandle_t xParsedDataQueue = NULL; // 用于在解析和处理之间传递数据的队列


/**
 * @brief 创建并启动HWCI 处理任务和解析结果队列
 *
 */
void vCreateHWCIProcessTask(void) // 创建 HWCI 任务的函数实现
{
    /* 创建解析后数据队列，容量为10 个ParsedDataPacket_t */ // 每行注释
    xParsedDataQueue = xQueueCreate(10, sizeof(ParsedDataPacket_t)); // 创建队列用于在解析和处理之间传递数据

    if (xParsedDataQueue == NULL) // 如果队列创建失败
    {
        QueueSendfmt(xSendDataQueue, 0, "创建 xParsedDataQueue 失败\r\n"); // 记录队列创建失败日志
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
                    case FUNCTION_CODE_DataPacket: // 功能码示例：0x02
                        if (gPendingRegisterContext.active &&
                            gPendingRegisterContext.rw == 1 &&
                            localPacket.ExecIndex != gPendingRegisterContext.execIndex)
                        {
                            ReplyPacket(localPacket.ExecIndex, REPLY_BUSY);
                            break;
                        }

                        if (localPacket.ExecIndex == TASK_ID_ESLCommands)
                        {
                            if (!(gPendingRegisterContext.active &&
                                  gPendingRegisterContext.execIndex == localPacket.ExecIndex &&
                                  gPendingRegisterContext.rw == 1 &&
                                  gPendingRegisterContext.number > 0))
                            {
                                ReplyPacket(localPacket.ExecIndex, REPLY_CONTEXT_MISMATCH);
                                break;
                            }

                            if (xTaskGetTickCount() > gPendingRegisterContext.deadlineTick)
                            {
                                ReplyPacket(localPacket.ExecIndex, REPLY_CONTEXT_TIMEOUT);
                                gPendingRegisterContext.active = 0;
                                break;
                            }

                            if (localPacket.DataLen != gPendingRegisterContext.number)
                            {
                                ReplyPacket(localPacket.ExecIndex, REPLY_LENGTH_MISMATCH);
                                gPendingRegisterContext.active = 0;
                                break;
                            }

                            E52bitWrite(gPendingRegisterContext.address,
                                        gPendingRegisterContext.number,
                                        localPacket.Data,
                                        localPacket.DataLen);
                            ReplyPacket(localPacket.ExecIndex, REPLY_OK);
                            gPendingRegisterContext.active = 0;
                            break;
                        }

                        if (localPacket.ExecIndex == TASK_ID_ESLReset)
                        {
                            SoftwareReset();
                            ReplyPacket(localPacket.ExecIndex, REPLY_OK);
                            break;
                        }

                        if (localPacket.ExecIndex == TASK_ID_ESLCheckBusy)
                        {
                            uint32_t timeoutMs = 500;
                            if (localPacket.DataLen >= 1)
                            {
                                timeoutMs = (uint32_t)localPacket.Data[0] * 100U;
                                if (timeoutMs == 0U)
                                {
                                    timeoutMs = 500U;
                                }
                            }

                            uint8_t ok = ESLCheckBusy(timeoutMs);
                            ReplyPacket(localPacket.ExecIndex, ok ? REPLY_OK : REPLY_BUSY);
                            break;
                        }

                        if (localPacket.ExecIndex == TASK_ID_SelectESLSPI)// 如果执行索引为SelectESLSPI
                        {
                            SelectESLSPI(localPacket.Data[0]); // 调用 SelectESLSPI 函数，传入SPIXLine 参数
                        }
                        if (localPacket.ExecIndex == TASK_ID_ICTypeSelect)// 如果执行索引为ICTypeSelect
                        {
                            ICTypeSelect(localPacket.Data[0]); // 调用 ICTypeSelect 函数，传入ICType 参数
                        }
                        if (localPacket.ExecIndex == TASK_ID_SelectInductor)// 如果执行索引为SelectInductor
                        {
                            SelectInductor(localPacket.Data[0]); // 调用 SelectInductor 函数，传入Inductor 参数
                        }
                        if (localPacket.ExecIndex == TASK_ID_SelectResistance)// 如果执行索引为SelectResistance
                        {
                            SelectResistance(localPacket.Data[0]); // 调用 SelectResistance 函数，传入Resistance 参数
                        }
                        if (localPacket.ExecIndex == TASK_ID_SetESLVDDVoltage)// 如果执行索引为SetESLVDDVoltage
                        {
                            TargetVDDVoltage = 2.3f + 0.1f * localPacket.Data[0]; // 设置目标电压（全局变量，PID会自动跃进）
                        }

                        ReplyPacket(localPacket.ExecIndex, REPLY_OK); // 按请求索引返回ACK
#if TEST_MODE
                        QueueSendfmt(xSendDataQueue, 0, "硬件配置成功, ExecIndex=%u, DataLen=%u\r\n", localPacket.ExecIndex, localPacket.DataLen); // 发送日志
#endif
                        break; // 退出该 case

                    case FUNCTION_CODE_CommandPacket: // 功能码示例：0x03
                        if (gPendingRegisterContext.active &&
                            gPendingRegisterContext.rw == 1 &&
                            localPacket.ExecIndex != gPendingRegisterContext.execIndex)
                        {
                            ReplyPacket(localPacket.ExecIndex, REPLY_BUSY);
                            break;
                        }

                        if (localPacket.ExecIndex == TASK_ID_ESLCommands) // 如果执行索引为ESLCommands
                        {
                            if (localPacket.DataLen < 3)
                            {
                                ReplyPacket(localPacket.ExecIndex, REPLY_PROTOCOL_ERROR);
                                gPendingRegisterContext.active = 0;
                                break;
                            }

                            uint8_t address = localPacket.Data[0];
                            uint8_t rw = localPacket.Data[1];
                            uint8_t number = localPacket.Data[2];

                            if (rw > 1)
                            {
                                ReplyPacket(localPacket.ExecIndex, REPLY_PROTOCOL_ERROR);
                                gPendingRegisterContext.active = 0;
                                break;
                            }

                            if (rw == 1 && number > 0)
                            {
                                if (gPendingRegisterContext.active && gPendingRegisterContext.rw == 1)
                                {
                                    if (gPendingRegisterContext.execIndex == localPacket.ExecIndex &&
                                        gPendingRegisterContext.address == address &&
                                        gPendingRegisterContext.number == number)
                                    {
                                        gPendingRegisterContext.deadlineTick = xTaskGetTickCount() + pdMS_TO_TICKS(REGISTER_WRITE_TRANSACTION_TIMEOUT_MS);
                                        ReplyPacket(localPacket.ExecIndex, REPLY_OK);
                                    }
                                    else
                                    {
                                        ReplyPacket(localPacket.ExecIndex, REPLY_BUSY);
                                    }
                                    break;
                                }

                                gPendingRegisterContext.active = 1;
                                gPendingRegisterContext.execIndex = localPacket.ExecIndex;
                                gPendingRegisterContext.address = address;
                                gPendingRegisterContext.rw = rw;
                                gPendingRegisterContext.number = number;
                                gPendingRegisterContext.deadlineTick = xTaskGetTickCount() + pdMS_TO_TICKS(REGISTER_WRITE_TRANSACTION_TIMEOUT_MS);

                                ESLCommands(localPacket.ExecIndex, address, rw, number); // 调用 ESLCommands 函数，传入数据和长度
                                ReplyPacket(localPacket.ExecIndex, REPLY_OK);
                                break;
                            }

                            ESLCommands(localPacket.ExecIndex, address, rw, number); // 调用 ESLCommands 函数，传入数据和长度
                            gPendingRegisterContext.active = 0;
                            if (rw == 1)
                            {
                                ReplyPacket(localPacket.ExecIndex, REPLY_OK);
                            }
                            /* rw==0 的读取路径由 E52bitRead 直接回 DataPacket，不再补 ACK */
                            break;
                        }

                        ReplyPacket(localPacket.ExecIndex, REPLY_OK); // 按请求索引返回ACK
                        break; // 退出该 case

                    default: // 未知功能码分支处理
                        QueueSendfmt(xSendDataQueue, 0, "未知功能码 0x%02X\r\n", localPacket.FunctionCode); // 发送未知功能码日志
                        break; // 退出默认分支
                }
            }
        }
    }
}


