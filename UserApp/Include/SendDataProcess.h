#ifndef SendLog_H
#define SendLog_H


#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <queue.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>    /* 若使用 vsnprintf/printf */
#include <stdarg.h>   /* for va_start / va_end */
#include "DPPTM.h"
#include "TaskList.h" // 功能码与应答码定义

/* 队列句柄，用于统一发送的存储和传递。 */
extern QueueHandle_t xSendDataQueue;

#define PRINT_QUEUE_ITEM_SIZE 128
/* 最大数据字段长度，根据协议最大包 64 字节：头1+功1+索引2+数据N+CRC1 <=64 */ // 每行注释
#define PACKET_MAX_DATA_LEN 59 /* 最大数据字节数 */ // 每行注释
#define PACKET_MAX_SIZE 64 /* 最大包长度（含 CRC） */ // 每行注释

void vCreateSendDataTask(void);
void vSendDataProcessTask(void *pvParameters);
void vCreateSendDataQueueTask(void *pvParameters);
BaseType_t QueueSendfmt(QueueHandle_t xQueue, TickType_t xTicksToWait, const char *fmt, ...);
BaseType_t SendBinaryToHost(const uint8_t *data, uint16_t len, TickType_t xTicksToWait);
BaseType_t SendBinaryToHostFront(const uint8_t *data, uint16_t len, TickType_t xTicksToWait);
uint8_t BuildReplyPacket(uint8_t functionCode, uint16_t execIndex, const uint8_t *data, uint8_t dataLen, uint8_t *outBuf, uint16_t *outLen);
void ReplyPacket(uint8_t reply); /* 执行成功应答固定为 FunctionCode=0x01, ExecIndex=0x0000 */


#endif // SendLog_H
