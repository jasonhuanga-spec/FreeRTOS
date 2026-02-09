#include "ESLComDri.h"

/**
 * @函数名称       复位
 * @说明           全局复位引脚。低电平复位。（正常情况下拉高）
 */
void ESL_RESET(void)
{
    HAL_GPIO_WritePin(RESET_GPIOX, RESET_GPIO_Pin_X, GPIO_PIN_SET);			
	HAL_Delay(100);//100mss
    HAL_GPIO_WritePin(RESET_GPIOX, RESET_GPIO_Pin_X, GPIO_PIN_RESET);   	
	HAL_Delay(100);								
    HAL_GPIO_WritePin(RESET_GPIOX, RESET_GPIO_Pin_X, GPIO_PIN_SET);			
    HAL_Delay(100);
}


/**
 * @函数名称       检查BUSY引脚
 * @说明           0忙，数据/命令正在转换；1：不忙，主机可以发送命令/数据
 */
void Check_Busy(void)
{ 
    while((HAL_GPIO_ReadPin(BUSY_GPIOX, BUSY_GPIO_Pin_X)) == 0);                   
}