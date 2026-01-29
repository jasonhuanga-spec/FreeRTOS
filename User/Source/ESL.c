#include "ESL.h"

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

/**
 * @函数名称       测试
 * @说明           验证功能
 */
void ESLtest(void)
{
    ESL_RESET();

    ESL4SPI_WriteCMD(0x4D);
    ESL4SPI_WriteDATA(0x78);

    ESL4SPI_WriteCMD(0x01);
	ESL4SPI_WriteDATA(0x07);
	ESL4SPI_WriteDATA(0x00);
	ESL4SPI_WriteDATA(0x16);
	ESL4SPI_WriteDATA(0x78);
	ESL4SPI_WriteDATA(0x2E);
	ESL4SPI_WriteDATA(0x16);
	
	ESL4SPI_WriteCMD(0x03);
	ESL4SPI_WriteDATA(0x10);
	ESL4SPI_WriteDATA(0x54);
	ESL4SPI_WriteDATA(0x44);
	
	ESL4SPI_WriteCMD(0x06);
	ESL4SPI_WriteDATA(0x0F);
	ESL4SPI_WriteDATA(0x0A);
	ESL4SPI_WriteDATA(0x2F);
	ESL4SPI_WriteDATA(0x25);
	ESL4SPI_WriteDATA(0x22);
	ESL4SPI_WriteDATA(0x2E);
	ESL4SPI_WriteDATA(0x1A);
	
	ESL4SPI_WriteCMD(0x61);
	ESL4SPI_WriteDATA(0x00);
	ESL4SPI_WriteDATA(0x98);
	ESL4SPI_WriteDATA(0x01);
	ESL4SPI_WriteDATA(0x28);
	
	ESL4SPI_WriteCMD(0x00);
	ESL4SPI_WriteDATA(0x03);
	ESL4SPI_WriteDATA(0x29);
	
	ESL4SPI_WriteCMD(0xE3);
	ESL4SPI_WriteDATA(0x22);
	
	ESL4SPI_WriteCMD(0xAE);
	ESL4SPI_WriteDATA(0x0F);
	
	ESL4SPI_WriteCMD(0xB6);
	ESL4SPI_WriteDATA(0x0F);
	
	ESL4SPI_WriteCMD(0x82);
	ESL4SPI_WriteDATA(0x96);
	
	ESL4SPI_WriteCMD(0x30);
	ESL4SPI_WriteDATA(0x08);
	
	ESL4SPI_WriteCMD(0xF0);
	ESL4SPI_WriteDATA(0x5F);
	
	ESL4SPI_WriteCMD(0xE9);
	ESL4SPI_WriteDATA(0x01);
	
	

	ESL4SPI_WriteCMD(0x10);
    for(int i = 0; i < 11248; i ++)
    {
        ESL4SPI_WriteDATA(0x00);
    }
    

    // ESL4SPI_WriteCMD(0x17);
	// ESL4SPI_WriteDATA(0xA5);

	ESL4SPI_WriteCMD(0x04); 
	Check_Busy();

	ESL4SPI_WriteCMD(0x12); 
	ESL4SPI_WriteDATA(0x00);
	Check_Busy();

    HAL_Delay(100);
}


/**
* @函数名称       E4读取ID
* @参数      	   无
* @返回值         无
* @说明           此函数仅适用于E5类型的IC
*/
void E4_Read_ID(void)
{

	ESL_RESET();
	
	ESL4SPI_WriteCMD(0x70);
	
	printf("ID = 0x%X ,0x%X ,0x%X\r\n", ESL4SPI_ReadDATA(),ESL4SPI_ReadDATA(),ESL4SPI_ReadDATA());

}
