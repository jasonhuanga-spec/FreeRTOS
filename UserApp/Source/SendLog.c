#include "SendLog.h"



// 全局变量定义
BaseType_t xCreateSendLogTaskReturned;
TaskHandle_t xCreateSendLogTaskHandle = NULL;
/* 队列句柄，用于接收数据的存储和传递。 */
QueueHandle_t xReceiveLogQueue = NULL;

/**
 * @brief 创建并启动发送日志任务。
 *
 * 创建任务并检查返回值；任务实现应为无限循环并在阻塞点等待数据。
 */
void vCreateSendLogTask(void)
{
    vCreateReceiveLogQueueTask(NULL);

    /* Create the task, storing the handle. */
    xCreateSendLogTaskReturned = xTaskCreate(
                   vSendLogProcessTask,                 /* Function that implements the task. */
                    "vSendLogProcessTask",              /* Text name for the task. */
                    512,                                    /* Stack size in words, not bytes. */
                    ( void * ) 1,                           /* Parameter passed into the task. */
                    tskIDLE_PRIORITY + 2,                   /* Priority at which the task is created. */
                    &xCreateSendLogTaskHandle );        /* Used to pass out the created task's handle. */

    if( xCreateSendLogTaskReturned == pdPASS )
    {
        
    }
}


/**
 * @brief 接收数据处理任务
 *
 * 该任务负责从系统的接收通道（例如 UART、SPI、USB 等）获取原始数据，
 * 对数据进行必要的解析与校验，并将处理后的消息/事件分发到应用层。
 */
void vSendLogProcessTask( void * pvParameters )
{
    /* 该参数值期望为 1，因为在下面对 xTaskCreate() 的调用中将 1 作为 pvParameters 传入。 */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );

    for (;;)
    {
        //放到循环中，不停更新这个数组
        char xReceiveLogQueueBuf[PRINT_QUEUE_ITEM_SIZE];

        if( xReceiveLogQueue != NULL )
            {
                /* 从已创建的队列接收一条消息，阻塞等待最多 portMAX_DELAY */
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
 * @brief 示例任务：创建一个队列用于接收数据
 *
 * 该任务创建一个队列以存储接收到的数据。实际应用中，
 * 任务应包含从外设接收数据并将其发送到队列的逻辑。
 */
void vCreateReceiveLogQueueTask( void *pvParameters )
{
    /* 创建一个用于保存接收数据包（ReceiveDataPacket_t）的队列，最多可容纳 10 个元素。 */
    /* 使用 PRINT_QUEUE_ITEM_SIZE 以确保队列项大小与发送/接收缓冲区一致，避免数据被截断或读取垃圾数据 */
    xReceiveLogQueue = xQueueCreate(10, PRINT_QUEUE_ITEM_SIZE);

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
  * @return BaseType_t 发送结果
  * 
  * // 无等待（不允许阻塞）
    QueueSendfmt(xPrintQueue, 0, "接收到数据: %02X", parsedDataPacket.Payload[0]);
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