#include "SendDataProcess.h" // 包含本模块的头文件，声明接口和常�?
#include "TaskList.h" // 包含功能码与应答码的定义（FUNCTION_CODE_Reply, REPLY_OK 等）



// 全局变量定义
BaseType_t xCreateSendLogTaskReturned;
TaskHandle_t xCreateSendLogTaskHandle = NULL;
/* 队列句柄，用于接收数据的存储和传递�?*/
QueueHandle_t xReceiveLogQueue = NULL;

/**
 * @brief 创建并启动发送日志任务�?
 *
 * 创建任务并检查返回值；任务实现应为无限循环并在阻塞点等待数据�?
 */
void vCreateSendLogTask(void)
{
    vCreateReceiveLogQueueTask(NULL);

    /* Create the task, storing the handle. */
    xCreateSendLogTaskReturned = xTaskCreate(
                   vSendLogProcessTask,                 /* Function that implements the task. */
                    "vSendLogProcessTask",              /* Text name for the task. */
                    256,                                    /* Stack size in words, not bytes. */
                    ( void * ) 1,                           /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,                   /* Priority at which the task is created. */
                    &xCreateSendLogTaskHandle );        /* Used to pass out the created task's handle. */

    if( xCreateSendLogTaskReturned == pdPASS )
    {
        
    }
}


/**
 * @brief 接收数据处理任务
 *
 * 该任务负责从系统的接收通道（例�?UART、SPI、USB 等）获取原始数据�?
 * 对数据进行必要的解析与校验，并将处理后的消息/事件分发到应用层�?
 */
void vSendLogProcessTask( void * pvParameters )
{
    /* 该参数值期望为 1，因为在下面�?xTaskCreate() 的调用中�?1 作为 pvParameters 传入�?*/
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );

    for (;;)
    {
        //放到循环中，不停更新这个数组
        char xReceiveLogQueueBuf[PRINT_QUEUE_ITEM_SIZE];

        if( xReceiveLogQueue != NULL )
            {
                /* 从已创建的队列接收一条消息，阻塞等待最�?portMAX_DELAY */
                if( xQueueReceive( xReceiveLogQueue, xReceiveLogQueueBuf, portMAX_DELAY ) == pdPASS )
                {
                    /* 计算实际字符串长度（不包括结尾的 \0 和垃圾数据） */
                    int actual_len = strlen(xReceiveLogQueueBuf);
                    
                    /* 只发送实际的字符串内容，而不是整个缓冲区 */
                    if( actual_len > 0 )    CDC_Transmit_FS((uint8_t*)xReceiveLogQueueBuf, actual_len);

                }
            }
    }
}   


/**
 * @brief 示例任务：创建一个队列用于接收数�?
 *
 * 该任务创建一个队列以存储接收到的数据。实际应用中�?
 * 任务应包含从外设接收数据并将其发送到队列的逻辑�?
 */
void vCreateReceiveLogQueueTask( void *pvParameters )
{
    /* 创建一个用于保存接收数据包（ReceiveDataPacket_t）的队列，最多可容纳 10 个元素�?*/
    /* 使用 PRINT_QUEUE_ITEM_SIZE 以确保队列项大小与发�?接收缓冲区一致，避免数据被截断或读取垃圾数据 */
    xReceiveLogQueue = xQueueCreate(20, PRINT_QUEUE_ITEM_SIZE);

    if( xReceiveLogQueue == NULL )
    {
        /* Queue was not created and must not be used. */
        
    }
}



 /** * @brief 格式化字符串并发送到队列
 *  * 
  * @param xQueue 队列句柄
  * @param xTicksToWait 等待时间
  * @param fmt 格式化字符串
  * @param ... 可变参数
  * @return BaseType_t 发送结�?
  * 
  * // 无等待（不允许阻塞）
    QueueSendfmt(xPrintQueue, 0, "接收到数�? %02X", parsedDataPacket.Payload[0]);
  */                    
BaseType_t QueueSendfmt(QueueHandle_t xQueue, TickType_t xTicksToWait, const char *fmt, ...)
{
    if (xQueue == NULL || fmt == NULL) {
        return pdFAIL;
    }

    char buf[PRINT_QUEUE_ITEM_SIZE];
    memset(buf, 0, sizeof(buf));

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    return xQueueSend(xQueue, buf, xTicksToWait);
}


/* @brief 计算数据包的 CRC8 校验�?
 * @param data 数据指针
 * @param length 数据长度
 * @return 计算得到�?CRC8 �?
 */
static uint8_t Packet_CRC8(const uint8_t *data, size_t length) 
{ // 每行注释
    uint8_t crc = 0x00; // 初始�?CRC
    for (size_t i = 0; i < length; ++i) { // 遍历所有字�?
        crc ^= data[i]; // 异或字节
        for (uint8_t bit = 0; bit < 8; ++bit) { // 逐位处理
            if (crc & 0x80) { // 检查最高位
                crc = (uint8_t)((crc << 1) ^ 0x07); // 左移并按多项式异�?
            } else {
                crc <<= 1; // 仅左�?
            }
        }
    }
    return crc; // 返回 CRC �?
}


/* 重构后的：构建应答包
 * 保持原有签名，增加更严格的参数与长度检查，避免缓冲区溢�?
 */
uint8_t BuildReplyPacket(uint8_t functionCode, uint16_t execIndex, const uint8_t *data, uint8_t dataLen, uint8_t *outBuf, uint16_t *outLen)
{
    if (outBuf == NULL || outLen == NULL) {
        return 1; // 空指�?
    }
    if (dataLen > PACKET_MAX_DATA_LEN) {
        return 2; // 数据域过�?
    }
    if (dataLen > 0 && data == NULL) {
        return 3; // dataLen>0 �?data �?NULL
    }

    /* 计算包长度并检查是否超过允许的最大包�?*/
    const uint16_t headerLen = 1;   /* 0xAA */
    const uint16_t funcLen   = 1;
    const uint16_t indexLen  = 2;
    const uint16_t crcLen    = 1;
    uint16_t totalLen = headerLen + funcLen + indexLen + dataLen + crcLen;

    if (totalLen > PACKET_MAX_SIZE) {
        return 4; // 超出最大缓冲区
    }

    /* 填充包内�?*/
    uint16_t pos = 0;
    outBuf[pos++] = 0xAA;                    /* 包头 */
    outBuf[pos++] = functionCode;            /* 功能�?*/
    outBuf[pos++] = (uint8_t)(execIndex >> 8); /* 执行索引高字�?*/
    outBuf[pos++] = (uint8_t)(execIndex & 0xFF); /* 执行索引低字�?*/

    if (dataLen > 0) {
        memcpy(&outBuf[pos], data, dataLen);
        pos += dataLen;
    }

    /* 计算 CRC（对包头至数据末尾计算，不包�?CRC 字节�?*/
    uint8_t crc = Packet_CRC8(outBuf, pos);
    outBuf[pos++] = crc;

    /* 返回实际长度 */
    *outLen = pos;
    return 0;
}


/* 重构后的：封装构建并发送应答包到日志队�?
 * - 原实现将二进制包放入 xReceiveLogQueue（文本队列），会�?strlen 截断，导致上位机未收到完整包
 * - 本实现直接通过 CDC_Transmit_FS 发送二进制包，并用文本日志报告发送结�?
 */
void ReplyPacket(uint8_t reply) // 参数改为 uint8_t 避免头文件循环依�?
{
    uint8_t outBuf[PACKET_MAX_SIZE]; // 定义输出缓冲区用于构建数据包
    uint16_t outLen = 0; // 存放构建后的包长�?
    uint16_t execIndex = (uint16_t)reply; // 将应答码放入执行索引字段�? 字节�?

    uint8_t buildRet = BuildReplyPacket((uint8_t)FUNCTION_CODE_Reply, execIndex, NULL, 0, outBuf, &outLen); // 调用构建函数
    if (buildRet != 0) { // 检查构建结果
        if (xReceiveLogQueue != NULL) { // 若日志队列存在则记录错误
            QueueSendfmt(xReceiveLogQueue, 0, "构建应答包失败，错误�?%u\r\n", buildRet); // 记录构建失败日志
        }
        return; // 构建失败，返回
    }

    /* 如需查看实际构建的字节序列，记录十六进制文本日志（非二进制队列） */
    if (xReceiveLogQueue != NULL) { // 若日志队列存在则格式化打印十六进制表�?
        char hexbuf[128]; // 缓冲区用于构建十六进制字符串
        int pos = 0; // 当前写入位置
        for (uint16_t i = 0; i < outLen && pos < (int)sizeof(hexbuf) - 4; ++i) { // 遍历包字节并格式化为字符�?
            pos += snprintf(&hexbuf[pos], sizeof(hexbuf) - pos, "%02X ", outBuf[i]); // 将每个字节格式化�?XX 空格
        }
        hexbuf[pos] = '\0'; // �?null 结尾
        QueueSendfmt(xReceiveLogQueue, 0, "构建完成�?u 字节）：%s\r\n", outLen, hexbuf); // 记录十六进制日志
    }

    /* 直接发送二进制包到上位机，避免文本队列截断 */
    const int maxRetries = 3; // 最大重试次�?
    uint8_t txRet = USBD_OK; // 发送返回�?
    for (int attempt = 0; attempt < maxRetries; ++attempt) { // 循环尝试发�?
        txRet = CDC_Transmit_FS(outBuf, outLen); // 调用 CDC 发送接口发送二进制�?
        if (xReceiveLogQueue != NULL) { // 记录每次尝试的返回值，便于诊断
            QueueSendfmt(xReceiveLogQueue, 0, "CDC_Transmit_FS 尝试=%d 返回=%u\r\n", attempt, txRet); // 记录返回�?
        }
        if (txRet == USBD_OK) { // 发送成功
            if (xReceiveLogQueue != NULL) { // 记录发送成功信息
                QueueSendfmt(xReceiveLogQueue, 0, "发送成功，长度=%u\r\n", outLen); // 发送成功日志
            }
            return; // 发送成功，返回
        }
        if (txRet == USBD_BUSY) { // USB 繁忙，等待短时延后重试
            vTaskDelay(pdMS_TO_TICKS(20)); // 延迟 20ms 后继续重试
            continue; // 继续重试循环
        }
        /* 其他错误则不再重试 */
        break; // 跳出重试循环
    }

    /* 如果到达此处表示发送失败，记录最终状态 */
    if (xReceiveLogQueue != NULL) { // 记录失败信息
        QueueSendfmt(xReceiveLogQueue, 0, "发送失败，最后错误码=%u\r\n", txRet); // 记录失败信息
    }
}





