#ifndef RECEIVEDATAPROCESS_H
#define RECEIVEDATAPROCESS_H

#include "main.h"
#include "cmsis_os.h"
#include <queue.h>
#include <string.h>  
#include "usbd_def.h"
#include "usbd_cdc.h"
#include "SendLog.h"

/* 定义最大接收数据包大小 */
#define MAX_RECEIVE_DATA_SIZE 64

/* 接收数据包结构体 */
typedef struct {
    uint8_t data[MAX_RECEIVE_DATA_SIZE];  /* 数据缓冲区 */
    uint16_t length;                      /* 数据长度 */
} ReceiveDataPacket_t;

//拆开接收到的数据包
typedef struct {
    uint8_t Header[2];          /* 数据包头 */
    uint8_t Command[2];         /* 命令字 */
    uint8_t Payload[59];        /* 有效载荷 */
    uint8_t Checksum;           /* 校验和CRC8 */
} ParsedDataPacket_t;

/* 队列句柄，用于接收数据的存储和传递。 */
extern QueueHandle_t xReceiveDataQueue;
// 拆开接收到的数据包
extern ParsedDataPacket_t parsedDataPacket;



void vCreateReceiveDataTask(void);
void vReceiveDataProcessTask( void * pvParameters );
void vCreateReceiveDataQueueTask( void *pvParameters );
uint8_t vReceiveDataQueueSendISRTask(uint8_t* Buf, uint32_t *Len);
void vParseReceivedDataPacket(void);


#endif // RECEIVEDATAPROCESS_H