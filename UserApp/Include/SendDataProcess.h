#ifndef SendLog_H
#define SendLog_H


#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <queue.h>
#include "usbd_cdc_if.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>    /* 若使用 vsnprintf/printf */
#include <stdarg.h>   /* for va_start / va_end */
#include "DPPTM.h"

/* 队列句柄，用于接收数据的存储和传递。 */
extern QueueHandle_t xReceiveLogQueue;

#define PRINT_QUEUE_ITEM_SIZE 128
/* 最大数据字段长度，根据协议最大包 64 字节：头1+功1+索引2+数据N+CRC1 <=64 */ // 每行注释
#define PACKET_MAX_DATA_LEN 59 /* 最大数据字节数 */ // 每行注释
#define PACKET_MAX_SIZE 64 /* 最大包长度（含 CRC） */ // 每行注释

void vCreateSendLogTask(void);
void vSendLogProcessTask( void * pvParameters );
void vCreateReceiveLogQueueTask( void *pvParameters );
BaseType_t QueueSendfmt(QueueHandle_t xQueue, TickType_t xTicksToWait, const char *fmt, ...);
uint8_t BuildReplyPacket(uint8_t functionCode, uint16_t execIndex, const uint8_t *data, uint8_t dataLen, uint8_t *outBuf, uint16_t *outLen);
void ReplyPacket(uint8_t reply); /* 参数使用 uint8_t 避免循环依赖，调用时传入 REPLY_OK 等值 */ // 每行注释


#endif // SendLog_H