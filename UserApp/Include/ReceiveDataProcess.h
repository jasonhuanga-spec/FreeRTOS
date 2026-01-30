#ifndef RECEIVEDATAPROCESS_H
#define RECEIVEDATAPROCESS_H

#include "main.h"
#include "cmsis_os.h"
#include <queue.h>
#include <string.h>  
#include "usbd_def.h"
#include "usbd_cdc.h"

/* 定义最大接收数据包大小 */
#define MAX_RECEIVE_DATA_SIZE 64

/* 接收数据包结构体 */
typedef struct {
    uint8_t data[MAX_RECEIVE_DATA_SIZE];  /* 数据缓冲区 */
    uint16_t length;                      /* 数据长度 */
} ReceiveDataPacket_t;

/* 队列句柄，用于接收数据的存储和传递。 */
extern QueueHandle_t xReceiveDataQueue;



void vCreateReceiveDataTask(void);
void vReceiveDataProcessTask( void * pvParameters );
void vCreateReceiveDataQueueTask( void *pvParameters );
uint8_t vReceiveDataQueueSendISRTask(uint8_t* Buf, uint32_t *Len);


#endif // RECEIVEDATAPROCESS_H