#include "Tasklist.h"



/*定义变量***********************************************************************************************************/

//创建实例
GetCDCDate_t GetCDCDate;


//任务列表
static Tasklist_t Tasklist = {
    .AdjustVDD = 0x01,                        //调整ESL模组使用的VDD
};

/*定义变量***********************************************************************************************************/



/**
 * @brief  根据数据包头，查找对应任务
 * @retval  CDC数据包的第0位0xFE和第1位是标志0xEF，第1位数据是任务索引
 */
static uint8_t SearchTask(void)
{
    //判断该数据包的包头是任务相关数据
    if ((GetCDCDate.GetCDCDateBuffer[0] = 0xFE) && (GetCDCDate.GetCDCDateBuffer[2] = 0xEF))
    {
        uint8_t TaskIndx = GetCDCDate.GetCDCDateBuffer[1];
        //返回任务索引
        return TaskIndx;
    }
}


/**
 * @brief  执行对应任务
 */
void TaskRun(void)
{
    //如果有新的数据没有处理，就开始匹配任务
    if (GetCDCDate.GetCDCDateFlag)
    {
        //调整ESL模组使用的VDD
        if (SearchTask() == Tasklist.AdjustVDD)
        {
            printf("开始读写数字电位器数据\r\n");
            DPPTM_RW(GetCDCDate.GetCDCDateBuffer[3], GetCDCDate.GetCDCDateBuffer[4], GetCDCDate.GetCDCDateBuffer[5]);
        }


        //数据处理完成，标志位置0，开始接收下一个数据包
        GetCDCDate.GetCDCDateFlag = 0;
    }
}


