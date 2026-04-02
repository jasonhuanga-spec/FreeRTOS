#ifndef RECEIVEDATAPROCESS_H
#define RECEIVEDATAPROCESS_H

#include "main.h"
#include "cmsis_os.h"
#include <queue.h>
#include <string.h>  
#include "SendDataProcess.h"
#include "TaskList.h"


/* 定义最大接收数据包大小 */
#define MAX_RECEIVE_DATA_SIZE 64

/* 接收数据包结构体 */
typedef struct {
    uint8_t data[MAX_RECEIVE_DATA_SIZE];  /* 数据缓冲区 */
    uint16_t length;                      /* 数据长度 */
} ReceiveDataPacket_t;

//拆开接收到的数据包
typedef struct { // 解析后的数据包结构体定义，包含协议字段
    uint8_t Header;        /* 数据包头，固定0xAA */ // 每行添加注释
    uint8_t FunctionCode;  /* 通信功能码，位于数组索引1 */ // 每行添加注释
    uint16_t ExecIndex;    /* 下位机执行索引，2字节，位于数组索引2-3 */ // 每行添加注释
    uint8_t DataLen;       /* 有效数据长度，动态表示数据字段长度 */ // 每行添加注释
    uint8_t Data[59];      /* 数据码，最大59字节，对应数组索引4-62 */ // 每行添加注释
    uint8_t CRC8;          /* 校验码CRC8，位于数据包最后一位 */ // 每行添加注释
} ParsedDataPacket_t; // 解析后的数据包类型定义

typedef struct {
    uint8_t active;
    uint16_t execIndex;
    uint8_t address;
    uint8_t rw;
    uint8_t number;
} PendingRevContext_t;

/* 队列句柄，用于接收数据的存储和传递。 */
extern QueueHandle_t xReceiveDataQueue;
// 拆开接收到的数据包
extern ParsedDataPacket_t parsedDataPacket;
extern PendingRevContext_t gPendingRevContext;

/* 解析后数据包的队列句柄，供 HWCI 任务从中接收解析后的数据包 */ // 每行注释
extern QueueHandle_t xParsedDataQueue; /* extern 声明：解析后数据队列 */ // 每行注释
/* HWCI 任务句柄的外部声明，供其他模块引用 */ // 每行注释
extern TaskHandle_t xHWCIProcessTaskHandle; /* extern 声明：HWCI 任务句柄 */ // 每行注释


void vCreateReceiveDataTask(void);
void vReceiveDataProcessTask( void * pvParameters );
void vCreateReceiveDataQueueTask( void *pvParameters );
uint8_t vReceiveDataQueueSendISRTask(uint8_t* Buf, uint32_t *Len);
void vParseReceivedDataPacket(const ReceiveDataPacket_t *packet);


#endif // RECEIVEDATAPROCESS_H
