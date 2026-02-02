#include "ReceiveDataProcess.h"


// 全局变量定义
BaseType_t xCreateReceiveDataTaskReturned;
TaskHandle_t xCreateReceiveDataTaskHandle = NULL;
/* 队列句柄，用于接收数据的存储和传递。 */
QueueHandle_t xReceiveDataQueue = NULL;
// 创建数据结构包含数据和长度信息
ReceiveDataPacket_t dataPacket;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
// 拆开接收到的数据包
ParsedDataPacket_t parsedDataPacket;



/**
 * @brief 创建并启动接收数据处理任务。
 *
 * 创建任务并检查返回值；任务实现应为无限循环并在阻塞点等待数据。
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
 * 该任务负责从系统的接收通道（例如 UART、SPI、USB 等）获取原始数据，
 * 对数据进行必要的解析与校验，并将处理后的消息/事件分发到应用层。
 */
void vReceiveDataProcessTask( void * pvParameters )
{
    /* 该参数值期望为 1，因为在下面对 xTaskCreate() 的调用中将 1 作为 pvParameters 传入。 */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    
    for (;;)
    {
        if( xReceiveDataQueue != NULL )
        {
            /* 从已创建的队列接收一条消息 */
            if( xQueueReceive( xReceiveDataQueue, &(dataPacket),(TickType_t) 0 ) == pdPASS )
            {
                /* xReceiveDataQueueRx now contains a copy of xMessage. */
                
                //打印接收到的原始数据（用于调试）
                #if TEST_MODE 
                    char hexStr[256]; // 足够容纳所有数据的十六进制表示
                    int offset = 0;
                    
                    offset += sprintf(hexStr + offset, "接收到数据[%d字节]: ", dataPacket.length);
                    
                    // 打印所有接收到的字节
                    for (uint16_t i = 0; i < dataPacket.length && i < MAX_RECEIVE_DATA_SIZE; i++)
                    {
                        offset += sprintf(hexStr + offset, "%02X ", dataPacket.data[i]);
                    }
                    
                    offset += sprintf(hexStr + offset, "\r\n");
                    
                    // 一次性发送完整的字符串到日志队列 
                    QueueSendfmt(xReceiveLogQueue, 0, "%s", hexStr);
                #endif
                
                
                /* 解析数据包 */
                vParseReceivedDataPacket();
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
void vCreateReceiveDataQueueTask( void *pvParameters )
{
    /* 创建一个用于保存接收数据包（ReceiveDataPacket_t）的队列，最多可容纳 10 个元素。 */
    xReceiveDataQueue = xQueueCreate( 10, sizeof(ReceiveDataPacket_t) );

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
    if (*Len <= MAX_RECEIVE_DATA_SIZE && xReceiveDataQueue != NULL)
    {
        // 先清零接收缓冲区
        memset(&dataPacket, 0, sizeof(dataPacket));

        // 拷贝数据到数据包结构中,数据持久化
        memcpy(dataPacket.data, Buf, *Len);
        dataPacket.length = *Len;

        // 将整个数据包结构发送到接收队列（不只是 data 数组，还包括 length）
        if (xQueueSendFromISR(xReceiveDataQueue, &dataPacket, &xHigherPriorityTaskWoken) == errQUEUE_FULL) 
            return (USBD_OK);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    return 0;
}



/**
 * @brief 解析接收到的数据包
 *
 */
void vParseReceivedDataPacket(void)
{
    
    
    // 发送解析成功的日志
    QueueSendfmt(xReceiveLogQueue, 0, "数据包解析成功，执行任务: %d \r\n");
}