#include "SendDataProcess.h" // 数据发送与封包模块


// 全局变量定义
BaseType_t xCreateSendDataTaskReturned;
TaskHandle_t xCreateSendDataTaskHandle = NULL;
QueueHandle_t xSendDataQueue = NULL; // 统一发送队列句柄

// 待发送项（文本或二进制）
typedef struct
{
    uint16_t len;              // 实际数据长度
    uint8_t  buf[PRINT_QUEUE_ITEM_SIZE]; // 数据内容
} SendDataItem_t;

/**
 * @brief 创建并启动发送数据任务（统一串行化 CDC 发送）
 */
void vCreateSendDataTask(void)
{
    vCreateSendDataQueueTask(NULL);

    xCreateSendDataTaskReturned = xTaskCreate(
        vSendDataProcessTask,
        "vSendDataProcessTask",
        256,
        (void *)1,
        tskIDLE_PRIORITY,
        &xCreateSendDataTaskHandle);
}

/**
 * @brief 发送数据任务：从队列取数据并串行调用 CDC_Transmit_FS
 */
void vSendDataProcessTask(void *pvParameters)
{
    configASSERT(((uint32_t)pvParameters) == 1);

    SendDataItem_t item;

    for (;;)
    {
        if (xSendDataQueue != NULL)
        {
            if (xQueueReceive(xSendDataQueue, &item, portMAX_DELAY) == pdPASS)
            {
                /* 等待 USB 空闲再发送，避免并发 BUSY */
                for (int attempt = 0; attempt < 5; ++attempt)
                {
                    uint32_t waitLoops = 0;
                    while (USBD_CDC_GetTxState() != 0 && waitLoops < 40)
                    {
                        vTaskDelay(pdMS_TO_TICKS(5));
                        waitLoops++;
                    }

                    if (USBD_CDC_GetTxState() != 0)
                    {
                        continue; // 继续下一次尝试
                    }

                    uint8_t txRet = CDC_Transmit_FS(item.buf, item.len);
                    if (txRet == USBD_OK)
                    {
                        break; // 发送成功
                    }

                    vTaskDelay(pdMS_TO_TICKS(5)); // 短暂等待再试
                }
            }
        }
    }
}

/**
 * @brief 创建待发送数据队列
 */
void vCreateSendDataQueueTask(void *pvParameters)
{
    (void)pvParameters;
    xSendDataQueue = xQueueCreate(20, sizeof(SendDataItem_t));
}

/**
 * @brief 格式化字符串并入统一发送队列
 */
BaseType_t QueueSendfmt(QueueHandle_t xQueue, TickType_t xTicksToWait, const char *fmt, ...)
{
    if (xQueue == NULL || fmt == NULL) {
        return pdFAIL;
    }

    SendDataItem_t item;
    memset(&item, 0, sizeof(item));

    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf((char*)item.buf, sizeof(item.buf), fmt, ap);
    va_end(ap);

    if (len <= 0) {
        return pdFAIL;
    }

    item.len = (len > (int)sizeof(item.buf)) ? (uint16_t)sizeof(item.buf) : (uint16_t)len;

    return xQueueSend(xQueue, &item, xTicksToWait);
}

/**
 * @brief 将二进制数据入统一发送队列
 */
BaseType_t SendBinaryToHost(const uint8_t *data, uint16_t len, TickType_t xTicksToWait)
{
    if (xSendDataQueue == NULL || data == NULL || len == 0) {
        return pdFAIL;
    }

    if (len > PRINT_QUEUE_ITEM_SIZE) {
        return pdFAIL; // 数据过长，避免溢出
    }

    SendDataItem_t item;
    item.len = len;
    memcpy(item.buf, data, len);

    return xQueueSend(xSendDataQueue, &item, xTicksToWait);
}

/* @brief 计算数据包的 CRC8 校验 */
static uint8_t Packet_CRC8(const uint8_t *data, size_t length)
{
    uint8_t crc = 0x00;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x07);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/* 构建应答/数据包 */
uint8_t BuildReplyPacket(uint8_t functionCode, uint16_t execIndex, const uint8_t *data, uint8_t dataLen, uint8_t *outBuf, uint16_t *outLen)
{
    if (outBuf == NULL || outLen == NULL) {
        return 1;
    }
    if (dataLen > PACKET_MAX_DATA_LEN) {
        return 2;
    }
    if (dataLen > 0 && data == NULL) {
        return 3;
    }

    const uint16_t headerLen = 1; /* 0xAA */
    const uint16_t funcLen   = 1;
    const uint16_t indexLen  = 2;
    const uint16_t crcLen    = 1;
    uint16_t totalLen = headerLen + funcLen + indexLen + dataLen + crcLen;

    if (totalLen > PACKET_MAX_SIZE) {
        return 4;
    }

    uint16_t pos = 0;
    outBuf[pos++] = 0xAA;
    outBuf[pos++] = functionCode;
    outBuf[pos++] = (uint8_t)(execIndex >> 8);
    outBuf[pos++] = (uint8_t)(execIndex & 0xFF);

    if (dataLen > 0) {
        memcpy(&outBuf[pos], data, dataLen);
        pos += dataLen;
    }

    uint8_t crc = Packet_CRC8(outBuf, pos);
    outBuf[pos++] = crc;

    *outLen = pos;
    return 0;
}

/* 封装并入队应答包 */
void ReplyPacket(uint8_t reply)
{
    uint8_t outBuf[PACKET_MAX_SIZE];
    uint16_t outLen = 0;
    uint16_t execIndex = (uint16_t)reply;

    uint8_t buildRet = BuildReplyPacket((uint8_t)FUNCTION_CODE_Reply, execIndex, NULL, 0, outBuf, &outLen);
    if (buildRet != 0) {
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "构建应答包失败，错误码=%u\r\n", buildRet);
        }
        return;
    }

    if (xSendDataQueue != NULL) {
        char hexbuf[128];
        int pos = 0;
        for (uint16_t i = 0; i < outLen && pos < (int)sizeof(hexbuf) - 4; ++i) {
            pos += snprintf(&hexbuf[pos], sizeof(hexbuf) - pos, "%02X ", outBuf[i]);
        }
        hexbuf[pos] = '\0';
        QueueSendfmt(xSendDataQueue, 0, "构建完成(%u 字节）：%s\r\n", outLen, hexbuf);
    }

    if (SendBinaryToHost(outBuf, outLen, pdMS_TO_TICKS(20)) != pdPASS)
    {
        if (xSendDataQueue != NULL) {
            QueueSendfmt(xSendDataQueue, 0, "应答包入队失败，长度=%u\r\n", outLen);
        }
    }
}
