#include "ReceiveDataProcess.h"


// 全局变量定义
BaseType_t xCreateReceiveDataTaskReturned;
TaskHandle_t xCreateReceiveDataTaskHandle = NULL;
/* 队列句柄，用于接收数据的存储和传递*/
QueueHandle_t xReceiveDataQueue = NULL;
// 创建数据结构包含数据和长度信息
// ReceiveDataPacket_t dataPacket; // REMOVED: Global variable causes race condition
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
// 拆开接收到的数据包
ParsedDataPacket_t parsedDataPacket;
PendingRevContext_t gPendingRevContext = {0};

static uint8_t rxStreamBuf[MAX_RECEIVE_DATA_SIZE * 4];
static uint16_t rxStreamLen = 0;

static uint8_t CalculateCRC8(const uint8_t *data, uint16_t length);
static void vParseReceivedDataStream(const ReceiveDataPacket_t *packet);



/**
 * @brief 创建并启动接收数据处理任务
 *
 * 创建任务并检查返回值；任务实现应为无限循环并在阻塞点等待数据
 */
void vCreateReceiveDataTask(void)
{
    vCreateReceiveDataQueueTask(NULL);

    /* Create the task, storing the handle. */
    xCreateReceiveDataTaskReturned = xTaskCreate(
                   vReceiveDataProcessTask,                      /* Function that implements the task. */
                    "vReceiveDataProcessTask",                   /* Text name for the task. */
                    256,                                         /* Stack size in words, not bytes. */
                    ( void * ) 1,                                /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,                            /* Highest possible priority */
                    &xCreateReceiveDataTaskHandle );             /* Used to pass out the created task's handle. */

    if( xCreateReceiveDataTaskReturned == pdPASS )
    {
        
    }
}


/**
 * @brief 接收数据处理任务
 *
 * 该任务负责从系统的接收通道（例如UART、SPI、USB 等）获取原始数据包
 * 对数据进行必要的解析与校验，并将处理后的消息/事件分发到应用层
 */
void vReceiveDataProcessTask( void * pvParameters )
{
    /* 该参数值期望为 1，因为在下面的xTaskCreate() 的调用中，1 作为 pvParameters 传入 */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    
    ReceiveDataPacket_t taskDataPacket; // Local variable for task

    for (;;)
    {
        if( xReceiveDataQueue != NULL )
        {
            /* 从已创建的队列接收一条消息 */
            if( xQueueReceive( xReceiveDataQueue, &(taskDataPacket),(TickType_t) 0 ) == pdPASS )
            {
                /* xReceiveDataQueueRx now contains a copy of xMessage. */
                
                //打印接收到的原始数据（用于调试）
                #if TEST_MODE 
                    char hexStr[256]; // 足够容纳所有数据的十六进制表示
                    int offset = 0;
                    
                    offset += sprintf(hexStr + offset, "接收到数据[%d字节]: ", taskDataPacket.length);
                    
                    // 打印所有接收到的数据字节
                    for (uint16_t i = 0; i < taskDataPacket.length; i++)
                    {
                        offset += sprintf(hexStr + offset, "%02X ", taskDataPacket.data[i]);
                    }
                    
                    offset += sprintf(hexStr + offset, "\r\n");
                    
                    // 一次性发送完整的字符串到日志队列 
                    QueueSendfmt(xSendDataQueue, 0, "%s", hexStr);
                #endif
                
                
                /* 解析数据流（兼容串口粘包/分片） */
                vParseReceivedDataStream(&taskDataPacket);
            }
        } 
    }
}



/**
 * @brief 示例任务：创建一个队列用于接收数据

 *
 * 该任务创建一个队列以存储接收到的数据。实际应用中
 * 任务应包含从外设接收数据并将其发送到队列的逻辑
 */
void vCreateReceiveDataQueueTask( void *pvParameters )
{
    /* 创建一个用于保存接收数据包（ReceiveDataPacket_t）的队列，最多可容纳 25 个元素 */
    xReceiveDataQueue = xQueueCreate( 25, sizeof(ReceiveDataPacket_t) );

    if( xReceiveDataQueue == NULL )
    {
        /* Queue was not created and must not be used. */
    }
 }



/** * @brief 在中断服务例程（ISR）中发送接收到的数据到接收队列
 *
 * @param Buf 指向接收到的数据缓冲区的指针
 * @param Len 指向数据长度的指针
 */
uint8_t vReceiveDataQueueSendISRTask(uint8_t* Buf, uint32_t *Len)
{
    if (Buf == NULL || Len == NULL || xReceiveDataQueue == NULL || *Len == 0)
    {
        return 0;
    }

    uint32_t offset = 0;
    uint32_t remaining = *Len;

    while (remaining > 0)
    {
        uint32_t chunkLen = (remaining > MAX_RECEIVE_DATA_SIZE) ? MAX_RECEIVE_DATA_SIZE : remaining;
        BaseType_t localTaskWoken = pdFALSE;
        ReceiveDataPacket_t isrPacket;

        memset(&isrPacket, 0, sizeof(isrPacket));
        memcpy(isrPacket.data, &Buf[offset], chunkLen);
        isrPacket.length = (uint16_t)chunkLen;

        if (xQueueSendFromISR(xReceiveDataQueue, &isrPacket, &localTaskWoken) == errQUEUE_FULL)
        {
            return 1;
        }

        if (localTaskWoken == pdTRUE)
        {
            xHigherPriorityTaskWoken = pdTRUE;
        }

        offset += chunkLen;
        remaining -= chunkLen;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    xHigherPriorityTaskWoken = pdFALSE;

    return 0;
}

static uint8_t IsHostRequestFunctionCode(uint8_t functionCode)
{
    return (functionCode == FUNCTION_CODE_DataPacket) ||
           (functionCode == FUNCTION_CODE_CommandPacket);
}

static uint16_t FindValidFrameLength(const uint8_t *buf, uint16_t available)
{
    if (buf == NULL || available < 5)
    {
        return 0;
    }

    if (buf[0] != 0xAA)
    {
        return 0;
    }

    if (!IsHostRequestFunctionCode(buf[1]))
    {
        return 0;
    }

    uint16_t maxTry = (available < MAX_RECEIVE_DATA_SIZE) ? available : MAX_RECEIVE_DATA_SIZE;
    for (uint16_t frameLen = 5; frameLen <= maxTry; frameLen++)
    {
        uint8_t crc = CalculateCRC8(buf, (uint16_t)(frameLen - 1));
        if (crc == buf[frameLen - 1])
        {
            return frameLen;
        }
    }

    return 0;
}

static void vParseReceivedDataStream(const ReceiveDataPacket_t *packet)
{
    if (packet == NULL || packet->length == 0)
    {
        return;
    }

    uint16_t appendLen = packet->length;
    if ((uint32_t)rxStreamLen + appendLen > sizeof(rxStreamBuf))
    {
        rxStreamLen = 0;
    }

    if (appendLen > sizeof(rxStreamBuf))
    {
        appendLen = (uint16_t)sizeof(rxStreamBuf);
    }

    memcpy(&rxStreamBuf[rxStreamLen], packet->data, appendLen);
    rxStreamLen = (uint16_t)(rxStreamLen + appendLen);

    while (rxStreamLen >= 5)
    {
        uint16_t start = 0;
        while (start < rxStreamLen && rxStreamBuf[start] != 0xAA)
        {
            start++;
        }

        if (start > 0)
        {
            memmove(rxStreamBuf, &rxStreamBuf[start], rxStreamLen - start);
            rxStreamLen = (uint16_t)(rxStreamLen - start);
        }

        if (rxStreamLen < 5)
        {
            break;
        }

        uint16_t frameLen = FindValidFrameLength(rxStreamBuf, rxStreamLen);
        if (frameLen == 0)
        {
            if (rxStreamLen > MAX_RECEIVE_DATA_SIZE)
            {
                memmove(rxStreamBuf, &rxStreamBuf[1], rxStreamLen - 1);
                rxStreamLen = (uint16_t)(rxStreamLen - 1);
                continue;
            }
            break;
        }

        ReceiveDataPacket_t framePacket;
        memset(&framePacket, 0, sizeof(framePacket));
        framePacket.length = frameLen;
        memcpy(framePacket.data, rxStreamBuf, frameLen);

        vParseReceivedDataPacket(&framePacket);

        if (rxStreamLen > frameLen)
        {
            memmove(rxStreamBuf, &rxStreamBuf[frameLen], rxStreamLen - frameLen);
        }
        rxStreamLen = (uint16_t)(rxStreamLen - frameLen);
    }
}



/**
 * @brief 计算 CRC-8 校验值（多项式0x07，初始值0x00）的封装函数
 */
static uint8_t CalculateCRC8(const uint8_t *data, uint16_t length) { /* 封装CRC8 计算函数定义 */ // 每行注释
    uint8_t crc = 0x00;                                           /* 初始CRC 值为 0x00 */ // 每行注释
    for (uint16_t i = 0; i < length; i++) {                       /* 遍历所有输入字节*/ // 每行注释
        crc ^= data[i];                                           /* 与当前字节异或*/ // 每行注释
        for (uint8_t bit = 0; bit < 8; bit++) {                   /* 对每个位进行处理 */ // 每行注释
            if (crc & 0x80)                                       /* 如果最高位为1 */ // 每行注释
            {
                crc = (uint8_t)((crc << 1) ^ 0x07);              /* 左移并按多项式异或*/ // 每行注释
            }
            else
            {
                crc <<= 1;                                       /* 仅左移一位*/ // 每行注释
            }
        }
    }
    return crc;                                                   /* 返回计算得到的CRC */ // 每行注释
}

/**
 * @brief 解析接收到的数据包
 *
 */
void vParseReceivedDataPacket(const ReceiveDataPacket_t *packet)
{
    uint8_t computedCRC = 0;                                        /* 计算得到的CRC8 */ // 每行注释
    uint8_t receivedCRC = 0;                                        /* 接收到的 CRC8 */ // 每行注释
    const uint8_t *buf = packet->data;                                 /* 指向接收数据缓冲区的指针 */ // 每行注释
    uint16_t len = packet->length;                                /* 接收数据的长度 */ // 每行注释

    /* 检查最小长度：包头(1)+功能码(1)+执行索引(2)+CRC(1) = 5 */ // 每行注释
    if (len < 5)                                                    /* 长度不足，直接返回 */ // 每行注释
    {
            QueueSendfmt(xSendDataQueue, 0, "解析失败：数据长度过短(%d)\r\n", len); /* 记录错误日志 */ // 每行注释
        return;                                                      /* 退出函数*/ // 每行注释
    }

    /* 验证包头是否�?0xAA */ // 每行注释
    if (buf[0] != 0xAA)                                              /* 包头错误，记录并返回 */ // 每行注释
    {
            QueueSendfmt(xSendDataQueue, 0, "解析失败：包头错误(0x%02X)\r\n", buf[0]); /* 日志包头错误 */ // 每行注释
        return;                                                      /* 退出函数*/ // 每行注释
    }

    /* 计算 CRC8：调用封装的 CalculateCRC8 函数，计算除最后一字节之外的CRC */ // 每行注释
    computedCRC = CalculateCRC8(buf, (uint16_t)(len - 1));          /* 通过封装函数计算 CRC */ // 每行注释

    receivedCRC = buf[len - 1];                                      /* 取出接收到的 CRC8 */ // 每行注释

    if (computedCRC != receivedCRC)                                  /* CRC 校验失败时记录日志并返回 */ // 每行注释
    {
            QueueSendfmt(xSendDataQueue, 0, "解析失败:CRC 校验错误(计算=0x%02X, 接收=0x%02X)\r\n", computedCRC, receivedCRC); /* CRC 错误日志 */ // 每行注释
        return;                                                      /* 退出函数*/ // 每行注释
    }

    /* CRC 校验通过，填充parsedDataPacket 结构体*/ // 每行注释
    parsedDataPacket.Header = buf[0];                                /* 填充包头 */ // 每行注释
    parsedDataPacket.FunctionCode = buf[1];                          /* 填充功能码 */ // 每行注释
    parsedDataPacket.ExecIndex = (uint16_t)((buf[2] << 8) | buf[3]);  /* 填充执行索引（按大端组合）*/ // 每行注释

    /* 计算数据段长度：总长度 - (包头1 + 功能码1 + 执行索引2 + CRC1) = len - 5 */ // 每行注释
    parsedDataPacket.DataLen = (uint8_t)((len > 5) ? (len - 5) : 0);  /* 填充数据长度 */ // 每行注释

    if (parsedDataPacket.DataLen > 0)                                /* 如果有数据则拷贝数据 */ // 每行注释
    {
        memcpy(parsedDataPacket.Data, &buf[4], parsedDataPacket.DataLen); /* 将数据段复制到结构体Data 数组 */ // 每行注释
    }

    parsedDataPacket.CRC8 = receivedCRC;                             /* 填充 CRC8 字段 */ // 每行注释

    /* ACK帧不进入业务执行队列，避免应答触发应答循环 */
    if (parsedDataPacket.FunctionCode == FUNCTION_CODE_Reply)
    {
        return;
    }

    /* 将解析后的数据包发送到 xParsedDataQueue，以供 HWCI 任务处理 */ // 每行注释
    if (xParsedDataQueue != NULL)                                     /* 检查解析队列是否已创建 */ // 每行注释
    {
        if (xQueueSend(xParsedDataQueue, &parsedDataPacket, (TickType_t)10) != pdPASS) /* 尝试发送解析包到队列，超时10个tick */ // 每行注释
        {
                QueueSendfmt(xSendDataQueue, 0, "解析后的数据发送到 xParsedDataQueue 失败\r\n"); /* 发送失败则记录日志 */ // 每行注释
        }
        else
        {
#if TEST_MODE
                QueueSendfmt(xSendDataQueue, 0, "解析后的数据已发送到 xParsedDataQueue\r\n"); /* 发送成功则记录日志 */ // 每行注释
#endif
        }
    }
    else
    {
            QueueSendfmt(xSendDataQueue, 0, "解析后的数据队列 xParsedDataQueue 未创建\r\n"); /* 队列不存在时记录日志 */ // 每行注释
    }

    /* 记录解析成功的日志，包括功能码、执行索引和数据长度 */ // 每行注释
#if TEST_MODE
        QueueSendfmt(xSendDataQueue, 0, "数据包解析成功: Func=0x%02X, ExecIdx=%u, DataLen=%u, CRC=0x%02X\r\n", parsedDataPacket.FunctionCode, parsedDataPacket.ExecIndex, parsedDataPacket.DataLen, parsedDataPacket.CRC8); /* 发送成功日志*/ // 每行注释
#endif
}
