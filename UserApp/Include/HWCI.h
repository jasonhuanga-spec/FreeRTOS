#ifndef HWCI_H
#define HWCI_H

#include "main.h"
#include "cmsis_os.h"
#include <queue.h>
#include <string.h>  
#include "SendLog.h"
#include "HWCIDri.h"
#include "TaskList.h"


/* 解析后数据包的队列句柄，供 HWCI 任务从中接收解析后的数据包 */ // 每行注释
extern QueueHandle_t xParsedDataQueue; /* extern 声明：解析后数据队列 */ // 每行注释

/* HWCI 任务句柄的外部声明，供其他模块引用 */ // 每行注释
extern TaskHandle_t xHWCIProcessTaskHandle; /* extern 声明：HWCI 任务句柄 */ // 每行注释

/* 创建 HWCI 处理任务的接口函数声明 */ // 每行注释
void vCreateHWCIProcessTask(void); /* 创建并启动 HWCI 处理任务 */ // 每行注释
void vHWCIProcessTask(void *pvParameters); /* HWCI 任务入口函数 */ // 每行注释



#endif // HWCI_H