#ifndef TASKLIST_H
#define TASKLIST_H

/* 任务 ID 列表：用于在系统中标识不同任务 */ // 每行注释
typedef enum {
    TASK_ID_NONE = 0, /* 无任务或未指定 */ // 每行注释
    TASK_ID_SelectESLSPI = 1, /* SelectESLSPI 任务，值为 1 */ // 每行注释
} TaskID_t; /* 任务 ID 枚举类型定义 */ // 每行注释

#endif /* TASKLIST_H */ // 每行注释