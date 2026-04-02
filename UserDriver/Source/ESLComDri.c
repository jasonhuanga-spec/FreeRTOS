#include "ESLComDri.h"

static void ESL_DelayMs(uint32_t ms)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
    else
    {
        HAL_Delay(ms);
    }
}

/**
 * @函数名称       复位
 * @说明           全局复位引脚。低电平复位。（正常情况下拉高）
 */
void ESL_RESET(void)
{
    HAL_GPIO_WritePin(RESET_GPIOX, RESET_GPIO_Pin_X, GPIO_PIN_SET);			
	ESL_DelayMs(100);//100mss
    HAL_GPIO_WritePin(RESET_GPIOX, RESET_GPIO_Pin_X, GPIO_PIN_RESET);   	
	ESL_DelayMs(100);								
    HAL_GPIO_WritePin(RESET_GPIOX, RESET_GPIO_Pin_X, GPIO_PIN_SET);			
    ESL_DelayMs(100);
}


/**
 * @函数名称       检查BUSY引脚
 * @说明           0忙，数据/命令正在转换；1：不忙，主机可以发送命令/数据
 */
void Check_Busy(void)
{ 
    while((HAL_GPIO_ReadPin(BUSY_GPIOX, BUSY_GPIO_Pin_X)) == 0);                   
}

uint8_t Check_BusyTimeout(uint32_t timeoutMs)
{
    uint32_t start = HAL_GetTick();
    while (HAL_GPIO_ReadPin(BUSY_GPIOX, BUSY_GPIO_Pin_X) == GPIO_PIN_RESET)
    {
        if ((HAL_GetTick() - start) >= timeoutMs)
        {
            return 0;
        }

        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
        {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    return 1;
}
