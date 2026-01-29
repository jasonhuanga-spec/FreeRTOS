#include "DPPTM.h"


/**
 * @函数名称       电子电位器抽头初始化
 * @说明           Date:是需要调整抽头位置的数据
 */
void DPPTMInitial(uint8_t Data)
{
    Soft_IIC_Start();                                                         //发送IIC起始信号                                              
    if(Soft_IIC_Write_Byte(0x7C))                                             //发送从机地址（0x7C），返回值表示是否发送成功
    {
        printf("发送数字电位器地址成功\r\n");
        if (Soft_IIC_Write_Byte(0x00))                                        //地址发送成功,发送命令字（0x00）
        {
            printf("发送数字电位器写命令成功\r\n");
            if (Soft_IIC_Write_Byte(Data))                                    //发送抽头调整数据
            {
                printf("向数字电位器写入数据成功,数据 = 0x%02X\r\n", Data);
            } 
    
        }
    }
    
    Soft_IIC_Stop();                                                           //发送IIC停止信号
}



/**
 * @brief  IIC读写函数
 * @param  Address: 数字电位器地址
 *         RW: 0是读数据，1是写数据
 *         DataNum: 待发送数据的字节长度
 */
void DPPTM_RW(uint8_t Address, uint8_t RW, uint8_t DataNum)
{
    if (RW == 0)
    {
        DPPTMRead(Address, RW, DataNum);
    }

    //要通过IIC写入数据
    if (RW == 1)
    {
        DPPTMWrite(Address, RW, DataNum);
    }
}


/**
 * @brief  IIC读数据函数
 * @param  Address: 数字电位器地址
 *         RW: 0是读数据，1是写数据
 *         DataNum: 待发送数据的字节长度
 */
void DPPTMRead(uint8_t Address, uint8_t RW, uint8_t DataNum)
{

}


/**
 * @brief  IIC写数据函数
 * @param  Address: 数字电位器地址
 *         RW: 0是读数据，1是写数据
 *         DataNum: 待发送数据的字节长度
 */
void DPPTMWrite(uint8_t Address, uint8_t RW, uint8_t DataNum)
{
    Soft_IIC_Start();                                                                                           //发送IIC起始信号                                              
    if(Soft_IIC_Write_Byte(Address) && (RW == 1))                                                               //发送从机地址（0x7C），返回值表示是否发送成功
    {
        printf("发送数字电位器地址成功\r\n");
        if (Soft_IIC_Write_Byte(0x00))                                                                          //地址发送成功,发送命令字（0x00）
        {
            printf("发送数字电位器写命令成功\r\n");
            for (int i = 0; i < DataNum; i++)                                                                   //一个CDC包，从第7个元素开始才是数据
            {
                if (Soft_IIC_Write_Byte(GetCDCDate.GetCDCDateBuffer[6 + i]))                                    //发送抽头调整数据
                {
                    printf("向数字电位器写入数据成功,数据 = 0x%02X\r\n", GetCDCDate.GetCDCDateBuffer[6 + i]);
                } 
            }
        }
    }
    
    Soft_IIC_Stop();                                                                                            //发送IIC停止信号
}

