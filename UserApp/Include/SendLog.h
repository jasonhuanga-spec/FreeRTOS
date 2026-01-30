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



/* 队列句柄，用于接收数据的存储和传递。 */
extern QueueHandle_t xReceiveLogQueue;

#define PRINT_QUEUE_ITEM_SIZE 128

void vCreateSendLogTask(void);
void vSendLogProcessTask( void * pvParameters );
void vCreateReceiveLogQueueTask( void *pvParameters );
BaseType_t QueueSendfmt(QueueHandle_t xQueue, TickType_t xTicksToWait, const char *fmt, ...);


#endif // SendLog_H