#include "ESL.h"
#include "SendDataProcess.h"  // 添加SendDataProcess.h以使用BuildReplyPacket等函数


/**
 * @函数名称       测试
 * @说明           验证功能
 */
void ESLtest(void)
{
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
 * @函数名称       E52bitRead
 * @说明           读取E52bit数据
*/
void E52bitRead(uint8_t Address, uint8_t Number)
{
    uint8_t readData[PACKET_MAX_DATA_LEN];  // 最大读取59字节
    uint8_t outBuf[PACKET_MAX_SIZE];
    uint16_t outLen = 0;

    ESL4SPI_WriteCMD(Address);  // 发送读取命令地址

    for (uint8_t i = 0; i < Number; i++)
    {
        readData[i] = ESL4SPI_ReadDATA();  // 读取数据并保存
    }

    // 构建数据包发送读取的数据
    uint8_t buildRet = BuildReplyPacket((uint8_t)FUNCTION_CODE_DataPacket, TASK_ID_ESLCommands, readData, Number, outBuf, &outLen);
    if (buildRet == 0)  // BuildReplyPacket成功返回0
    {
        SendBinaryToHost(outBuf, outLen, pdMS_TO_TICKS(20));
    }
}



/******************** E52bitWrite ********************
 * @函数名称       E52bitWrite
 * @说明           写入E52bit数据
 * @参数           Address: 写入地址
 *                Number: 写入数据数量
 * @返回值         无
 * @备注           这里示例写入索引值，实际应用中应替换为需要写入的数据
 */
void E52bitWrite(uint8_t Address, uint8_t Number)
{
	ESL4SPI_WriteCMD(Address);  // 发送写入命令地址

	for (uint8_t i = 0; i < Number; i++)
	{
		ESL4SPI_WriteDATA(i);  // 示例写入数据（这里写入索引值，实际应用中应替换为需要写入的数据）
	}
}


/******************** ESLCommands ********************
 * @函数名称       ESLCommands
 * @说明           处理ESL相关命令，根据RW参数决定读写操作
 * @参数           Address: 读写地址
 *                RW: 读写标志（0表示读，1表示写）
 *                Number: 读写数据数量
 * @返回值         无
 * @备注           根据RW参数调用E52bitRead或E52bitWrite函数执行具体的读写操作
 */
void ESLCommands(uint8_t Address, uint8_t RW, uint8_t Number)
{
	switch(RW)
	{
		case 0:			//读
			E52bitRead(Address, Number);
			break;
		
		case 1:			//写	
			E52bitWrite(Address, Number);
			break;
	
		default:
			break;
	}
}
