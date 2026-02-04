#ifndef TASKLIST_H
#define TASKLIST_H



/* 功能码列表：用于区分不同的通信功能 */ // 每行注释
typedef enum {
    FUNCTION_CODE_Reply = 1, /* Reply 功能码，值为 1 */ // 每行注释
    FUNCTION_CODE_DataPacket = 2, /* DataPacket 功能码，值为 2 */ // 每行注释
    FUNCTION_CODE_CommandPacket = 3, /* CommandPacket 功能码，值为 3 */ // 每行注释
} FunctionCode_t; /* 任务优先级枚举类型定义 */ // 每行注释



/* 任务 ID 列表：用于在系统中标识不同任务 */ // 每行注释
typedef enum {
    TASK_ID_NONE = 0, /* 无任务或未指定 */ // 每行注释
    TASK_ID_SelectESLSPI = 1, /* SelectESLSPI 任务，值为 1 */ // 每行注释
    TASK_ID_ICTypeSelect = 2, /* ICTypeSelect 任务，值为 2 */ // 每行注释
    TASK_ID_SelectInductor = 3, /* SelectInductor 任务，值为 3 */ // 每行注释
    TASK_ID_SelectResistance = 4, /* SelectResistance 任务，值为 4 */ // 每行注释
} TaskID_t; /* 任务 ID 枚举类型定义 */ // 每行注释



/* 应答码枚举：用于上位机和下位机之间的应答表示 */ // 每行注释
typedef enum {
    REPLY_OK = 0 /* 操作成功应答 */ // 每行注释
} ReplyCode_t; /* 应答码枚举类型定义 */ // 每行注释

#endif /* TASKLIST_H */ // 每行注释