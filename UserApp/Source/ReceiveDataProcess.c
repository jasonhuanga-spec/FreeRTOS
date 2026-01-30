#include "ReceiveDataProcess.h"


/* 队列句柄，用于接收数据的存储和传递。 */
QueueHandle_t xReceiveDataQueue = NULL;
// 创建数据结构包含数据和长度信息
ReceiveDataPacket_t dataPacket;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;



/**
 * @brief 创建并启动接收数据处理任务。
 *
 * 创建任务并检查返回值；任务实现应为无限循环并在阻塞点等待数据。
 */
void vCreateReceiveDataTask(void)
{
    BaseType_t xCreateReceiveDataTaskReturned;
    TaskHandle_t xCreateReceiveDataTaskHandle = NULL;

    vCreateReceiveDataQueueTask( NULL );

    /* Create the task, storing the handle. */
    xCreateReceiveDataTaskReturned = xTaskCreate(
                   vReceiveDataProcessTask,                 /* Function that implements the task. */
                    "NAME",                                 /* Text name for the task. */
                    256,                                    /* Stack size in words, not bytes. */
                    ( void * ) 1,                           /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,                       /* Priority at which the task is created. */
                    &xCreateReceiveDataTaskHandle );        /* Used to pass out the created task's handle. */

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

    uint8_t xReceiveDataQueueRx[ MAX_RECEIVE_DATA_SIZE ];
    
    for (;;)
    {
        if( xReceiveDataQueue != NULL )
        {
            /* 从已创建的队列接收一条消息 */
            if( xQueueReceive( xReceiveDataQueue, &( xReceiveDataQueueRx ),( TickType_t ) 10 ) == pdPASS )
            {
                /* xReceiveDataQueueRx now contains a copy of xMessage. */
                printf("接收到数据: ");
                for (int i = 0; i < MAX_RECEIVE_DATA_SIZE; i++)
                {
                    printf("%02X ", xReceiveDataQueueRx[i]);
                }
                printf("\r\n");
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
    xReceiveDataQueue = xQueueCreate( 10, sizeof(dataPacket.data) );

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
        memset(&dataPacket, 0, sizeof(ReceiveDataPacket_t));

        // 拷贝数据到数据包结构中,数据持久化
        memcpy(dataPacket.data, Buf, *Len);
        dataPacket.length = *Len;

        // 将数据包发送到接收队列;本质是「尝试发送，通过结果判断是否有空间」，无需额外的空间检查步骤
        if (xQueueSendFromISR(xReceiveDataQueue, &dataPacket.data, &xHigherPriorityTaskWoken) == errQUEUE_FULL) 
            return (USBD_OK);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    return 0;
}

